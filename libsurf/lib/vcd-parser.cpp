#include "vcd-parser.h"
#include "common-internal.h"
#include "vcd-lexer.h"
#include <utils.h>

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
    std::vector<int> decls;
    const char *position;
};

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

struct comment {
    SCA rule = LEXY_LIT("$comment") + dsl::any + LEXY_LIT("$end");
};

struct tick {
    using tick_ty = decltype(Tick{}.tick);
    SCA rule      = dsl::lit_c<'#'> + dsl::integer<tick_ty>;
    SCA value     = lexy::construct<Tick>;
};

struct value_change {
    SCA rule  = LEXY_LIT("TODO");
    SCA value = lexy::noop;
};

struct sim_cmd {
    SCA rule  = dsl::peek(dsl::lit_c<'#'>) >> dsl::p<tick> | dsl::p<value_change>;
    SCA value = lexy::callback<SimCmd>(
        []() {
            return SimCmd{};
        },
        [](Tick tick) {
            return SimCmd{};
        });
};

SCA end_defs_decl_pair = LEXY_LIT("$enddefinitions") + dsl::token(ws) + LEXY_LIT("$end");

struct decl_list {
    SCA rule = [] {
        auto num = dsl::p<decimal_number>;
        return dsl::terminator(dsl::token(end_defs_decl_pair)).list(num);
    }();
    SCA value = lexy::as_list<std::vector<int>>;
};

struct decl_list_remaining {
    SCA rule  = dsl::p<decl_list> + dsl::position;
    SCA value = lexy::callback<VCDDeclRuleRes>([](std::vector<int> &&decls, const char *position) {
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
    SCA value = lexy::callback<Document>([](std::vector<int> &&decls, std::vector<SimCmd> &&cmds) {
        return Document{.declarations = std::move(decls), .sim_cmds = std::move(cmds)};
    });
    SCA whitespace = ws;
};

}; // namespace grammar

}; // namespace

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
    return {.declarations = std::move(decls_ret.decls),
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

    // std::string trace;
    // lexy::trace_to<grammar::document>(std::back_insert_iterator(trace), input,
    //                                   {lexy::visualize_fancy});
    // fmt::print("{:s}\n", trace);

    Document res;
    VCDParserDeclRet decls_ret;

    try {
        decls_ret        = parse_vcd_declarations(vcd_str, path);
        res.declarations = std::move(decls_ret.decls);
    } catch (const VCDDeclParseError &decl_parse_error) {
        fmt::print(stderr, "Error parsing VCD declarations:\n{:s}\n", decl_parse_error.what());
    }
    try {
        res.sim_cmds = parse_vcd_sim_cmds(decls_ret.remaining, path);
    } catch (const VCDSimCmdsParseError &cmds_parse_error) {
        fmt::print(stderr, "Error parsing VCD simulation commands:\n{:s}\n",
                   cmds_parse_error.what());
    }
    fmt::print("2-step decls: {}\n", fmt::join(res.declarations, ", "));
    fmt::print("2-step cmds: {}\n", fmt::join(res.sim_cmds, ", "));

    auto doc_res =
        lexy::parse<grammar::vcd_document>(input, lexy_ext::report_error.path(path.c_str()));
    fmt::print("1-step doc success: {}\n", doc_res.is_success());
    if (doc_res.is_success()) {
        res = std::move(doc_res.value());
    }
    fmt::print("1-step decls: {}\n", fmt::join(res.declarations, ", "));
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
