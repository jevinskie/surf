#include "vcd-parser.h"
#include "common-internal.h"
#include "vcd-lexer.h"
#include <utils.h>

#include <sstream>

#include <lexy/action/parse.hpp>         // lexy::parse
#include <lexy/action/parse_as_tree.hpp> // lexy::parse_as_tree
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

namespace grammar {
namespace dsl = lexy::dsl;

SCA ws = dsl::whitespace(dsl::ascii::space);

struct decimal_number {
    SCA rule  = dsl::sign + dsl::integer<int>;
    SCA value = lexy::as_integer<int>;
};

struct word {
    SCA rule  = dsl::identifier(dsl::ascii::word);
    SCA value = lexy::as_string<std::string>;
};

struct sim_cmd {
    SCA rule  = dsl::p<word>;
    SCA value = lexy::forward<std::string>;
};

SCA end_defs_decl_pair = LEXY_LIT("$enddefinitions") + dsl::token(ws) + LEXY_LIT("$end");

struct decl_list {
    SCA rule = [] {
        auto num = dsl::p<decimal_number>;
        return dsl::terminator(dsl::token(end_defs_decl_pair)).list(num);
    }();

    SCA value = lexy::as_list<std::vector<int>>;
};

struct opt_decl_list {
    SCA rule  = dsl::opt(dsl::else_ >> dsl::try_(dsl::p<decl_list>));
    SCA value = lexy::as_list<std::vector<int>> >>
                lexy::callback<std::vector<int>>(
                    [](std::vector<int> &&decls) {
                        fmt::print("opt_decl_list vector<int>\n");
                        return LEXY_MOV(decls);
                    },
                    [](void) {
                        fmt::print("opt_decl_list void\n");
                        return std::vector<int>{};
                    },
                    [](lexy::nullopt decls) {
                        fmt::print("opt_decl_list nullopt\n");
                        return std::vector<int>{};
                    });
};

SCA end_cmds_decl_pair = LEXY_LIT("$endcmds") + dsl::token(ws) + LEXY_LIT("$end");

struct sim_cmd_list {
    SCA rule = dsl::list(dsl::peek_not(dsl::eof) >> dsl::p<word>);
    // SCA rule = dsl::list(dsl::p<word>);
    // SCA rule  = dsl::terminator(dsl::token(end_cmds_decl_pair)).list(dsl::p<word>);
    SCA value = lexy::as_list<std::vector<std::string>>;
};

struct opt_sim_cmd_list {
    SCA rule  = dsl::opt(dsl::else_ >> dsl::try_(dsl::p<sim_cmd_list>));
    SCA value = lexy::as_list<std::vector<std::string>> >>
                lexy::callback<std::vector<std::string>>(
                    [](std::vector<std::string> &&sim_cmds) {
                        fmt::print("opt_sim_cmd_list vector<string>\n");
                        return LEXY_MOV(sim_cmds);
                    },
                    [](void) {
                        fmt::print("opt_sim_cmd_list void\n");
                        return std::vector<std::string>{};
                    },
                    [](lexy::nullopt sim_cmds) {
                        fmt::print("opt_sim_cmd_list nullopt\n");
                        return std::vector<std::string>{};
                    });
};

struct decls_and_cmds {
    SCA rule = dsl::p<decl_list> + dsl::p<sim_cmd_list> + dsl::eof;
    SCA value =
        lexy::callback<Document>([](std::vector<int> &&decls, std::vector<std::string> &&sim_cmds) {
            return Document{.nums = LEXY_MOV(decls), .words = LEXY_MOV(sim_cmds)};
        });
    SCA whitespace = ws;
};

struct just_decls {
    SCA rule       = dsl::p<decl_list> + dsl::eof;
    SCA value      = lexy::callback<Document>([](std::vector<int> &&decls) {
        return Document{.nums = LEXY_MOV(decls)};
    });
    SCA whitespace = ws;
};

struct just_cmds {
    SCA rule       = dsl::p<sim_cmd_list> + dsl::eof;
    SCA value      = lexy::callback<Document>([](std::vector<std::string> &&sim_cmds) {
        return Document{.words = LEXY_MOV(sim_cmds)};
    });
    SCA whitespace = ws;
};

struct empty {
    SCA rule       = dsl::eof;
    SCA value      = lexy::callback<Document>([]() {
        return Document{};
    });
    SCA whitespace = ws;
};

#if 0
struct document {
    SCA ds_and_cs = dsl::p<decls_and_cmds>;
    SCA ds        = dsl::p<just_decls>;
    SCA cs        = dsl::p<just_cmds>;
    SCA ef        = dsl::p<empty>;

#if 0    
    SCA a = dsl::else_ >> dsl::if_(dsl::peek(decls_and_cmds) >> decls_and_cmds);
    SCA b = a | dsl::else_ >> dsl::if_(dsl::peek(just_decls) >> just_decls);
    SCA c = b | dsl::else_ >> dsl::if_(dsl::peek(just_cmds) >> just_cmds);
    SCA d = c | dsl::else_ >> empty;
    SCA d = c + dsl::if_(dsl::peek(dsl::eof));
    SCA d = c | empty;
    SCA c = b | empty;
    SCA a = dsl::else_ >> decls_and_cmds;
    SCA b = a | dsl::else_ >> just_decls;
    SCA c = b | dsl::else_ >> just_cmds;
    SCA d = c | dsl::else_ >> empty;
#endif

#if 0
    SCA a = dsl::peek(decls_and_cmds) >> decls_and_cmds;
    SCA b = a | dsl::peek(just_decls) >> just_decls;
    SCA c = b | dsl::peek(just_cmds) >> just_cmds;
    SCA d = c | dsl::else_ >> empty;
    SCA rule = d;
#endif

#if 0
    SCA a    = dsl::else_ >> dsl::try_(ds_and_cs);
    SCA b    = a | dsl::else_ >> dsl::try_(dsl::else_ >> ds);
    SCA c    = b | dsl::else_ >> dsl::try_(dsl::else_ >> cs);
    SCA d    = c | dsl::else_ >> ef;
    SCA rule = d;
#endif

#if 1
    SCA a    = dsl::else_ >> dsl::try_(dsl::else_ >> ds_and_cs);
    SCA b    = a | dsl::else_ >> dsl::try_(dsl::else_ >> ds);
    SCA c    = b | dsl::else_ >> dsl::try_(dsl::else_ >> cs);
    SCA d    = c | dsl::else_ >> ef;
    SCA rule = dsl::try_(dsl::p<opt_decl_list>) + dsl::try_(dsl::p<opt_sim_cmd_list>) + dsl::eof;

    // SCA rule = d;

#endif

    // SCA rule = dsl::try_(decls) + dsl::try_(cmds) + dsl::eof;
    // SCA rule = dsl::peek(decls_and_cmds) >> decls_and_cmds | dsl::else_ >>
    // just_decls | dsl::else_ >> just_cmds | dsl::else_ >> empty;

    // SCA value = lexy::callback<Document>();

    SCA value = lexy::callback<Document>(
        [](std::vector<int> &&decls) {
            // DUMP_STACK("single decls arg");
            fmt::print("single decls arg\n");
            return Document{.nums = LEXY_MOV(decls)};
        },
        [](std::vector<std::string> &&sim_cmds) {
            fmt::print("single sim_cmds arg\n");
            return Document{.words = LEXY_MOV(sim_cmds)};
        },
        [](std::vector<int> &&decls, std::vector<std::string> &&sim_cmds) {
            fmt::print("decls and sim_cmds\n");
            return Document{.nums = LEXY_MOV(decls), .words = LEXY_MOV(sim_cmds)};
        },
        [](std::vector<int> &&decls, lexy::nullopt sim_cmds) {
            fmt::print("just decls\n");
            return Document{.nums = LEXY_MOV(decls)};
        },
        [](lexy::nullopt decls, std::vector<std::string> &&sim_cmds) {
            fmt::print("just sim_cmds\n");
            return Document{.words = LEXY_MOV(decls)};
        },
        [](lexy::nullopt decls, lexy::nullopt sim_cmds) {
            fmt::print("two nullopt\n");
            return Document{};
        },
        [](lexy::nullopt wtf) {
            fmt::print("single nullopt wtf\n");
            return Document{};
        },
        [](Document &&doc) {
            fmt::print("doc rvalue\n");
            return LEXY_MOV(doc);
        },
        []() {
            // DUMP_STACK("void");
            fmt::print("void\n");
            return Document{};
        });
};
#endif

struct skip_whitespace {
    SCA whitespace = dsl::ascii::space;
};

#if 1
struct document : lexy::scan_production<Document> {
    SCA whitespace = dsl::ascii::space;

    template <typename Reader, typename Context>
    static scan_result scan(lexy::rule_scanner<Context, Reader> &scanner) {
        const auto orig_input = scanner.remaining_input();
        lexy::scan_result<Document> result;

        fmt::print("a\n");
        std::string ds_and_cs_err;
        auto ds_and_cs_res = lexy::parse<grammar::decls_and_cmds>(
            orig_input, lexy_ext::report_error.to(std::back_insert_iterator(ds_and_cs_err)));
        if (ds_and_cs_res.is_success()) {
            fmt::print("scan ds_and_cs match\n");
            return scan_result{LEXY_MOV(ds_and_cs_res.value())};
        }

        fmt::print("b\n");
        std::string ds_err;
        auto ds_res = lexy::parse<grammar::just_decls>(
            orig_input, lexy_ext::report_error.to(std::back_insert_iterator(ds_err)));
        if (ds_res.is_success()) {
            fmt::print("scan ds match\n");
            return scan_result{LEXY_MOV(ds_res.value())};
        }

        fmt::print("c\n");
        std::string cs_err;
        auto cs_res = lexy::parse<grammar::just_cmds>(
            orig_input, lexy_ext::report_error.to(std::back_insert_iterator(cs_err)));
        if (cs_res.is_success()) {
            fmt::print("scan cs match\n");
            return scan_result{LEXY_MOV(cs_res.value())};
        }

        fmt::print("d\n");
        std::string empty_err;
        auto empty_res = lexy::parse<grammar::empty>(
            orig_input, lexy_ext::report_error.to(std::back_insert_iterator(empty_err)));
        if (empty_res.is_success()) {
            fmt::print("scan empty match\n");
            return scan_result{LEXY_MOV(empty_res.value())};
        }

        fmt::print("e\n");
        return lexy::scan_failed;
    }
};
#endif

}; // namespace grammar

}; // namespace

VCDParserDeclRet parse_vcd_declarations(std::string_view decls_str) {
    auto cursor = decls_str.data();
    return {.decls = {}, .remaining = {cursor, decls_str.size() - (cursor - decls_str.data())}};
}

std::vector<SimCmd> parse_vcd_sim_cmds(std::string_view sim_cmds_str) {
    std::vector<SimCmd> cmds;
    return cmds;
}

Document parse_vcd_document(std::string_view vcd_str, const fs::path &path) {
    auto declsret = parse_vcd_declarations(vcd_str);
    return {.declarations = LEXY_MOV(declsret.decls),
            .sim_cmds     = parse_vcd_sim_cmds(declsret.remaining)};
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

    // std::string trace;
    // lexy::trace_to<grammar::document>(std::back_insert_iterator(trace), input,
    //                                   {lexy::visualize_fancy});
    // fmt::print("{:s}\n", trace);

    auto doc_res = lexy::parse<grammar::document>(input, lexy_ext::report_error.path(path.c_str()));
    fmt::print("doc: is_success: {} is_error: {} is_recovered_error: {} is_fatal_error: {} "
               "has_value: {}\n",
               doc_res.is_success(), doc_res.is_error(), doc_res.is_recovered_error(),
               doc_res.is_fatal_error(), doc_res.has_value());
    if (doc_res.has_value()) {
        auto doc = LEXY_MOV(doc_res.value());
        fmt::print("doc:\n");
        if (doc.nums) {
            fmt::print("nums: {}\n", fmt::join(*doc.nums, ", "));
        } else {
            fmt::print("nums: EMPTY\n");
        }
        if (doc.words) {
            fmt::print("words: {}\n", fmt::join(*doc.words, ", "));
        } else {
            fmt::print("words: EMPTY\n");
        }
    } else {
        fmt::print("doc: no value!\n");
    }
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
