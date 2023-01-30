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
    SCA value      = lexy::as_list<std::vector<int>>;
    SCA whitespace = ws;
};

struct ws_eof {
    SCA rule = dsl::token(ws) + dsl::eof;
};

struct sim_cmd_eof_list {
    SCA rule       = dsl::list(dsl::peek_not(dsl::eof) >> dsl::p<word>);
    SCA value      = lexy::as_list<std::vector<std::string>>;
    SCA whitespace = ws;
};

}; // namespace grammar

}; // namespace

VCDParserDeclRet parse_vcd_declarations(std::string_view decls_str, fs::path path) {
    VCDParserDeclRet res;
    auto input = lexy::string_input<lexy::ascii_encoding>(decls_str);
    std::string decls_err;
    auto scanner = lexy::scan(
        input, lexy_ext::report_error.to(std::back_insert_iterator(decls_err)).path(path.c_str()));
    auto decls_res = scanner.parse<grammar::decl_list>();
    if (!scanner || !decls_res.has_value()) {
        throw std::logic_error("VCD declaration (header) parsing failed.\n" + decls_err);
    }
    auto cstr_rest = scanner.remaining_input().reader().position();
    return VCDParserDeclRet{
        .decls     = std::move(decls_res.value()),
        .remaining = {cstr_rest, decls_str.size() - (cstr_rest - decls_str.data())}};
}

std::vector<std::string> parse_vcd_sim_cmds(std::string_view sim_cmds_str, fs::path path) {
    auto input = lexy::string_input<lexy::ascii_encoding>(sim_cmds_str);
    std::string cmds_err;
    auto scanner = lexy::scan(
        input, lexy_ext::report_error.to(std::back_insert_iterator(cmds_err)).path(path.c_str()));
    auto cmds_res = scanner.parse<grammar::sim_cmd_eof_list>();
    if (!scanner || !cmds_res.has_value()) {
        throw std::logic_error("VCD commands (changes) parsing failed.\n" + cmds_err);
    }
    return std::move(cmds_res.value());
}

Document parse_vcd_document(std::string_view vcd_str, const fs::path &path) {
    auto decls_ret = parse_vcd_declarations(vcd_str, path);
    return {.declarations = std::move(decls_ret.decls),
            .sim_cmds     = parse_vcd_sim_cmds(decls_ret.remaining)};
}

#if 1
void parse_vcd_document_test(std::string_view vcd_str, const fs::path &path) {
    auto input = lexy::string_input<lexy::ascii_encoding>(vcd_str);
    Document res;
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

    VCDParserDeclRet decls_ret;

    try {
        decls_ret        = parse_vcd_declarations(vcd_str, path);
        res.declarations = std::move(decls_ret.decls);
    } catch (const VCDDeclParseError &decl_parse_error) {
        fmt::print(stderr, "Error parsing VCD declarations:\n{:s}\n", decl_parse_error.what());
    }
    try {
        res.sim_cmds = parse_vcd_sim_cmds(decls_ret.remaining, path);
    } catch (const VCDDeclParseError &decl_parse_error) {
        fmt::print(stderr, "Error parsing VCD simulation commands:\n{:s}\n",
                   decl_parse_error.what());
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
