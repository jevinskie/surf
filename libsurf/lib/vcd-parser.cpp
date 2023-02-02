#include "vcd-parser.h"
#include "common-internal.h"
#include "vcd-lexer.h"
#include <utils.h>

#include <charconv>

#include <lexy/action/parse.hpp>         // lexy::parse
#include <lexy/action/parse_as_tree.hpp> // lexy::parse_as_tree
#include <lexy/action/scan.hpp>          // lexy::scan
#include <lexy/action/trace.hpp>         // lexy::trace_to
#include <lexy/callback.hpp>             // value callbacks
#include <lexy/dsl.hpp>                  // lexy::dsl::*
#include <lexy/input/string_input.hpp>   // lexy::string_input
#include <lexy/parse_tree.hpp>           // lexy::parse_tree_for

#include <lexy_ext/report_error.hpp> // lexy_ext::report_error

#include <concurrencpp/concurrencpp.h>

// need combination_duplicate for decl parsing

#define SCA static constexpr auto

using namespace VCDTypes;

namespace surf {

namespace {

struct VCDDeclRuleRes {
    std::vector<Declaration> decls;
    const char *position;
};

namespace grammar {
namespace dsl = lexy::dsl;

using str_lex = lexy::string_lexeme<lexy::ascii_encoding>;
SCA to_sv(str_lex lexeme) {
    return std::string_view{lexeme.data(), lexeme.size()};
}
std::string to_string(str_lex lexeme) {
    return std::string(lexeme.data(), lexeme.size());
}
SCA lex2str_cb = lexy::callback<std::string>([](str_lex lexeme) {
    return to_string(lexeme);
});

SCA ws           = dsl::whitespace(dsl::ascii::space);
SCA all_chars    = dsl::ascii::character;
SCA non_ws_chars = dsl::ascii::character - dsl::ascii::space;
SCA val_chars    = LEXY_ASCII_ONE_OF("01xXzZ");
SCA end_term     = dsl::terminator(dsl::token(ws + LEXY_LIT("$end")));
SCA ws_term      = dsl::terminator(dsl::token(ws));
SCA all_cap      = dsl::capture(all_chars);

SCA cap_tok(auto rule) {
    return dsl::capture(dsl::token(rule));
}

struct decimal_number {
    SCA rule  = dsl::sign + dsl::integer<int>;
    SCA value = lexy::as_integer<int>;
};

struct comment {
    SCA rule  = LEXY_LIT("$comment") + dsl::no_whitespace(end_term.list(all_cap));
    SCA value = lexy::as_string<std::string> >> lexy::construct<Comment>;
};

struct tick {
    SCA rule  = dsl::lit_c<'#'> + dsl::integer<decltype(Tick{}.tick)>;
    SCA value = lexy::construct<Tick>;
};

struct id {
    SCA rule  = dsl::identifier(non_ws_chars);
    SCA value = lexy::as_string<std::string>;
};

struct scalar_value {
    SCA rule  = cap_tok(val_chars);
    SCA value = lexy::callback<ScalarValue>([](auto lexeme) {
        assert(lexeme.size() == 1);
        return ScalarValue(lexeme.data()[0]);
    });
};

struct binary_number {
    SCA rule  = LEXY_ASCII_ONE_OF("bB") + dsl::integer<decltype(BinaryNum{}.num), dsl::binary>;
    SCA value = lexy::construct<BinaryNum>;
};

struct real_number {
    SCA rule = LEXY_ASCII_ONE_OF("rR") + cap_tok(dsl::sign + dsl::integer<uint64_t>) +
               dsl::opt(dsl::token(dsl::period) >> cap_tok(dsl::integer<uint64_t>));
    SCA value = lexy::callback<RealNum>(
        [](auto leading_lex, lexy::nullopt) {
            auto whole_str =
                std::string(leading_lex.data(), leading_lex.data() + leading_lex.size());
            char *end;
            errno    = 0;
            double d = strtod(whole_str.c_str(), &end);
            if (errno == ERANGE || d < std::numeric_limits<double>::lowest() ||
                d > std::numeric_limits<double>::max()) {
                throw std::range_error(fmt::format("real_number out of range: '{:s}'", whole_str));
            }
            return RealNum{.num = d};
        },
        [](auto leading_lex, str_lex trailing_lex) {
            auto whole_str =
                std::string(leading_lex.data(), trailing_lex.data() + trailing_lex.size());
            char *end;
            errno    = 0;
            double d = strtod(whole_str.c_str(), &end);
            if (errno == ERANGE || d < std::numeric_limits<double>::lowest() ||
                d > std::numeric_limits<double>::max()) {
                throw std::range_error(fmt::format("real_number out of range: '{:s}'", whole_str));
            }
            return RealNum{.num = d};
        });
};

struct any_value {
    struct val_error {
        SCA name = "bad value";
    };
    SCA rule = (dsl::peek(LEXY_ASCII_ONE_OF("bB")) >> dsl::p<binary_number>) |
               (dsl::peek(val_chars) >> dsl::p<scalar_value>) |
               (dsl::peek(LEXY_ASCII_ONE_OF("rR")) >> dsl::p<real_number>) |
               (dsl::else_ >> dsl::error<val_error>);
    SCA value = lexy::callback<Value>(
        [](BinaryNum bnum) {
            return Value{bnum};
        },
        [](RealNum rnum) {
            return Value{rnum};
        },
        [](ScalarValue sv) {
            return Value{sv};
        });
};

struct value_change {
    SCA rule  = dsl::p<any_value> + dsl::p<id>;
    SCA value = lexy::callback<Change>([](Value value, std::string &&id) {
        return Change{.value = value, .id = std::move(id)};
    });
};

struct sim_cmd {
    SCA rule = (dsl::peek(dsl::lit_c<'#'>) >> dsl::p<tick>) |
               (dsl::peek(LEXY_LIT("$comment")) >> dsl::p<comment>) |
               (dsl::else_ >> dsl::p<value_change>);
    SCA value = lexy::callback<SimCmd>(
        [](Comment &&comment) {
            return SimCmd{std::move(comment)};
        },
        [](Tick &&tick) {
            return SimCmd{std::move(tick)};
        },
        [](Change &&change) {
            return SimCmd{std::move(change)};
        });
};

struct date {
    SCA rule  = LEXY_LIT("$date") + dsl::no_whitespace(end_term.list(dsl::capture(all_chars)));
    SCA value = lexy::as_string<std::string> >> lexy::construct<Date>;
};

struct version {
    SCA rule  = LEXY_LIT("$version") + dsl::no_whitespace(end_term.list(dsl::capture(all_chars)));
    SCA value = lexy::as_string<std::string> >> lexy::construct<Version>;
};

struct time_number {
    struct bad_time_num {
        SCA name = "bad time number - should be one of: 1, 10, 100";
    };
    SCA rule = cap_tok(LEXY_LITERAL_SET(LEXY_LIT("100"), LEXY_LIT("10"), LEXY_LIT("1"))) |
               (dsl::else_ >> dsl::error<bad_time_num>);
    SCA value = lexy::callback<TimeNumber>([](str_lex lexeme) {
        auto str      = "n"s + to_string(lexeme);
        auto time_num = magic_enum::enum_cast<TimeNumber>(str);
        if (!time_num.has_value()) {
            throw std::domain_error(fmt::format("Bad time number: '{:s}'", str));
        }
        return time_num.value();
    });
};

struct time_unit {
    struct bad_time_unit {
        SCA name = "bad time unit - should be one of: s, ms, us, ns, ps, fs";
    };
    SCA rule = cap_tok(LEXY_LITERAL_SET(LEXY_LIT("fs"), LEXY_LIT("ps"), LEXY_LIT("ns"),
                                        LEXY_LIT("us"), LEXY_LIT("ms"), LEXY_LIT("s"))) |
               (dsl::else_ >> dsl::error<bad_time_unit>);
    SCA value = lexy::callback<TimeUnit>([](str_lex lexeme) {
        auto str       = to_sv(lexeme);
        auto time_unit = magic_enum::enum_cast<TimeUnit>(str);
        if (!time_unit.has_value()) {
            throw std::domain_error(fmt::format("Bad time unit: '{:s}'", str));
        }
        return time_unit.value();
    });
};

struct timescale {
    SCA rule  = LEXY_LIT("$timescale") + dsl::p<time_number> + dsl::p<time_unit> + LEXY_LIT("$end");
    SCA value = lexy::construct<Timescale>;
};

struct scope_type {
    struct bad_scope_type {
        SCA name = "bad scope type - should be one of: begin, fork, function, module, task";
    };
    SCA rule = cap_tok(LEXY_LITERAL_SET(LEXY_LIT("begin"), LEXY_LIT("fork"), LEXY_LIT("function"),
                                        LEXY_LIT("module"), LEXY_LIT("task"))) |
               (dsl::else_ >> dsl::error<bad_scope_type>);
    SCA value = lexy::callback<ScopeType>([](str_lex lexeme) {
        auto str        = to_sv(lexeme);
        auto scope_type = magic_enum::enum_cast<ScopeType>(str);
        if (!scope_type.has_value()) {
            throw std::domain_error(fmt::format("Bad scope type: '{:s}'", str));
        }
        return scope_type.value();
    });
};

struct var_type {
    struct bad_var_type {
        SCA name =
            "bad var type - should be one of: event, integer, parameter, real, realtime, reg, "
            "supply0, supply1, time, tri, triand, trior, trireg, tri0, tri1, wand, wire, wor";
    };
    SCA rule = cap_tok(LEXY_LITERAL_SET(
                   LEXY_LIT("event"), LEXY_LIT("integer"), LEXY_LIT("parameter"), LEXY_LIT("real"),
                   LEXY_LIT("realtime"), LEXY_LIT("reg"), LEXY_LIT("supply0"), LEXY_LIT("supply1"),
                   LEXY_LIT("time"), LEXY_LIT("tri"), LEXY_LIT("triand"), LEXY_LIT("trior"),
                   LEXY_LIT("trireg"), LEXY_LIT("tri0"), LEXY_LIT("tri1"), LEXY_LIT("wand"),
                   LEXY_LIT("wire"), LEXY_LIT("wor"))) |
               (dsl::else_ >> dsl::error<bad_var_type>);
    SCA value = lexy::callback<VarType>([](str_lex lexeme) {
        auto str      = to_sv(lexeme);
        auto var_type = magic_enum::enum_cast<ScopeType>(str);
        if (!var_type.has_value()) {
            throw std::domain_error(fmt::format("Bad var type: '{:s}'", str));
        }
        return var_type.value();
    });
};

struct scope_id {
    SCA rule  = dsl::identifier(non_ws_chars);
    SCA value = lexy::as_string<std::string>;
};

SCA upscope_decl_pair = LEXY_LIT("$upscope") + dsl::token(ws) + LEXY_LIT("$end");

struct scope {
    SCA rule = LEXY_LIT("$scope") + dsl::p<scope_type> + dsl::p<scope_id> + LEXY_LIT("$end") +
               upscope_decl_pair;
    SCA value = lexy::callback<Scope>([](ScopeType scope_type, std::string &&scope_id) {
        return Scope{.id = std::move(scope_id), .type = std::move(scope_type)};
    });
};

struct declaration {
    struct bad_decl {
        SCA name = "bad declaration";
    };
    SCA rule = (dsl::peek(LEXY_LIT("$comment")) >> dsl::p<comment>) |
               (dsl::peek(LEXY_LIT("$date")) >> dsl::p<date>) |
               (dsl::peek(LEXY_LIT("$version")) >> dsl::p<version>) |
               (dsl::peek(LEXY_LIT("$timescale")) >> dsl::p<timescale>) |
               (dsl::peek(LEXY_LIT("$scope")) >> dsl::p<scope>) |
               (dsl::else_ >> dsl::error<bad_decl>);
    SCA value = lexy::callback<Declaration>(
        [](Comment &&comment) {
            return Declaration{std::move(comment)};
        },
        [](Date &&date) {
            return Declaration{std::move(date)};
        },
        [](Version &&version) {
            return Declaration{std::move(version)};
        },
        [](Timescale timescale) {
            return Declaration{timescale};
        },
        [](Scope &&scope) {
            return Declaration{std::move(scope)};
        });
};

SCA end_defs_decl_pair = LEXY_LIT("$enddefinitions") + dsl::token(ws) + LEXY_LIT("$end");

struct decl_list {
    SCA rule = [] {
        auto decl = dsl::p<declaration>;
        return dsl::terminator(dsl::token(end_defs_decl_pair)).list(decl);
    }();
    SCA value = lexy::as_list<std::vector<Declaration>>;
};

struct decl_list_remaining {
    SCA rule = dsl::p<decl_list> + dsl::position;
    SCA value =
        lexy::callback<VCDDeclRuleRes>([](std::vector<Declaration> &&decls, const char *position) {
            return VCDDeclRuleRes{.decls = std::move(decls), .position = position};
        });
    SCA whitespace = ws;
};

struct sim_cmd_eof_list {
    SCA rule       = dsl::terminator(dsl::eof).list(dsl::p<sim_cmd>);
    SCA value      = lexy::as_list<std::vector<SimCmd>>;
    SCA whitespace = ws;
};

struct vcd_document {
    SCA rule  = dsl::p<decl_list> + dsl::p<sim_cmd_eof_list>;
    SCA value = lexy::callback<DocumentRawDecls>(
        [](std::vector<Declaration> &&decls, std::vector<SimCmd> &&cmds) {
            return DocumentRawDecls{.declarations = std::move(decls), .sim_cmds = std::move(cmds)};
        });
    SCA whitespace = ws;
};

}; // namespace grammar

}; // namespace

VCDTypes::Declarations decls_from_decl_list(const std::vector<VCDTypes::Declaration> &decl_list) {
    return {};
}

static lexy::visualization_options realize_opts(std::optional<lexy::visualization_options> opts) {
    if (opts) {
        return *opts;
    }
    if (can_use_term_colors()) {
        return {lexy::visualize_fancy};
    }
    return {};
}

VCDParserDeclRet parse_vcd_declarations(std::string_view decls_str, fs::path path,
                                        std::optional<lexy::visualization_options> opts) {
    auto input = lexy::string_input<lexy::ascii_encoding>(decls_str);
    std::string decls_err;
    auto decls_parse_res = lexy::parse<grammar::decl_list_remaining>(
        input, lexy_ext::report_error.path(path.c_str())
                   .to(std::back_insert_iterator(decls_err))
                   .opts(realize_opts(opts)));
    if (!decls_parse_res.is_success() || !decls_parse_res.has_value()) {
        throw std::logic_error("VCD declaration (header) parsing failed.\n" + decls_err);
    }
    auto decls_val = decls_parse_res.value();
    auto remaining = std::string_view(decls_val.position,
                                      decls_str.size() - (decls_val.position - decls_str.data()));
    return VCDParserDeclRet{.decls = std::move(decls_val.decls), .remaining = remaining};
}

std::vector<SimCmd> parse_vcd_sim_cmds(std::string_view sim_cmds_str, fs::path path,
                                       std::optional<lexy::visualization_options> opts) {
    auto input = lexy::string_input<lexy::ascii_encoding>(sim_cmds_str);
    std::string cmds_err;
    auto cmds_parse_res =
        lexy::parse<grammar::sim_cmd_eof_list>(input, lexy_ext::report_error.path(path.c_str())
                                                          .to(std::back_insert_iterator(cmds_err))
                                                          .opts(realize_opts(opts)));
    if (!cmds_parse_res.is_success() || !cmds_parse_res.has_value()) {
        throw std::logic_error("VCD commands (changes) parsing failed.\n" + cmds_err);
    }
    return std::move(cmds_parse_res.value());
}

Document parse_vcd_document(std::string_view vcd_str, const fs::path &path,
                            std::optional<lexy::visualization_options> opts) {
    auto decls_ret = parse_vcd_declarations(vcd_str, path, opts);
    return {.declarations = decls_from_decl_list(std::move(decls_ret.decls)),
            .sim_cmds     = parse_vcd_sim_cmds(decls_ret.remaining, path, opts)};
}

#if 1
void parse_vcd_document_test(std::string_view vcd_str, const fs::path &path) {
    auto input = lexy::string_input<lexy::ascii_encoding>(vcd_str);
    // auto validate_res =
    //     lexy::validate<grammar::document>(input, lexy_ext::report_error.path(path.c_str()));
    // fmt::print("is_error: {}\n", validate_res.is_error());

    // lexy::parse_tree_for<decltype(input)> parse_tree;
    // auto tree_res = lexy::parse_as_tree<grammar::document>(parse_tree, input, lexy::noop);
    // if (tree_res.is_success()) {
    //     lexy::visualize(stdout, parse_tree, {lexy::visualize_fancy});
    // } else {
    //     fmt::print("parse_as_tree error: {}\n", tree_res.errors());
    // }

    fmt::print("sizeof(Value): {}\n", sizeof(Value));
    fmt::print("sizeof(SimCmd): {}\n", sizeof(SimCmd));
    fmt::print("sizeof(ScalarValue): {}\n", sizeof(ScalarValue));

    std::string trace;
    lexy::trace_to<grammar::vcd_document>(std::back_insert_iterator(trace), input,
                                          {lexy::visualize_fancy});
    fmt::print("trace:\n\n{:s}\n", trace);

    Document res;
    VCDParserDeclRet decls_ret;

    try {
        decls_ret        = parse_vcd_declarations(vcd_str, path);
        res.declarations = decls_from_decl_list(decls_ret.decls);
    } catch (const VCDDeclParseError &decl_parse_error) {
        fmt::print(stderr, "Error parsing VCD declarations:\n{:s}\n", decl_parse_error.what());
    }
    try {
        res.sim_cmds = parse_vcd_sim_cmds(decls_ret.remaining, path);
    } catch (const VCDSimCmdsParseError &cmds_parse_error) {
        fmt::print(stderr, "Error parsing VCD simulation commands:\n{:s}\n",
                   cmds_parse_error.what());
    }
    fmt::print("2-step raw decls: {}\n", fmt::join(decls_ret.decls, ", "));
    fmt::print("2-step decls: {}\n", res.declarations);
    fmt::print("2-step cmds: {}\n", fmt::join(res.sim_cmds, ", "));

    auto doc_res =
        lexy::parse<grammar::vcd_document>(input, lexy_ext::report_error.path(path.c_str()));
    fmt::print("1-step doc success: {}\n", doc_res.is_success());
    if (doc_res.is_success()) {
        auto doc_val = std::move(doc_res.value());
        res = Document{.declarations = decls_from_decl_list(std::move(doc_val.declarations)),
                       .sim_cmds     = std::move(doc_val.sim_cmds)};
    }
    fmt::print("1-step decls: {}\n", res.declarations);
    fmt::print("1-step cmds: {}\n", fmt::join(res.sim_cmds, ", "));
}
#endif

#if 0
void parse_vcd_document_test(std::string_view vcd_str, const fs::path &path) {
    (void)path;
    VCDLexer lexer;
    const auto base = vcd_str.data();
    for (const auto p : lexer.parse(vcd_str.data())) {
        fmt::print("cursor: {}\n", p - base);
    }
}
#endif

}; // namespace surf
