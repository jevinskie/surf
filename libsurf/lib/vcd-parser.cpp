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
    SCA rule       = dsl::terminator(dsl::eof).list(dsl::p<word>);
    SCA value      = lexy::as_list<std::vector<std::string>>;
    SCA whitespace = ws;
};

struct vcd_document {
    SCA rule = dsl::p<decl_list> + dsl::p<sim_cmd_eof_list>;
    SCA value =
        lexy::callback<Document>([](std::vector<int> &&decls, std::vector<std::string> &&cmds) {
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
    VCDParserDeclRet res;
    auto input = lexy::string_input<lexy::ascii_encoding>(decls_str);
    std::string decls_err;
    auto scanner = lexy::scan(input, lexy_ext::report_error.to(std::back_insert_iterator(decls_err))
                                         .path(path.c_str())
                                         .opts(realize_opts(opts)));
    auto decls_res = scanner.parse<grammar::decl_list>();
    if (!scanner || !decls_res.has_value()) {
        throw std::logic_error("VCD declaration (header) parsing failed.\n" + decls_err);
    }
    fmt::print("decls_error: {}\n", decls_err);
    fmt::print("pos: {}\n", scanner.position());
    auto cstr_rest = scanner.position();
    return VCDParserDeclRet{
        .decls     = std::move(decls_res.value()),
        .remaining = {cstr_rest, decls_str.size() - (cstr_rest - decls_str.data())}};
}

std::vector<std::string> parse_vcd_sim_cmds(std::string_view sim_cmds_str, fs::path path,
                                            std::optional<lexy::visualization_options> opts) {
    auto input = lexy::string_input<lexy::ascii_encoding>(sim_cmds_str);
    std::string cmds_err;
    auto scanner  = lexy::scan(input, lexy_ext::report_error.to(std::back_insert_iterator(cmds_err))
                                          .path(path.c_str())
                                          .opts(realize_opts(opts)));
    auto cmds_res = scanner.parse<grammar::sim_cmd_eof_list>();
    fmt::print("cmds_err: {}\n", cmds_err);
    fmt::print("pos: {}\n", scanner.position());
    if (!scanner || cmds_res.has_value()) {
        throw std::logic_error("VCD commands (changes) parsing failed.\n" + cmds_err);
    }
    return std::move(cmds_res.value());
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
    fmt::print("remaining: {:s}\n", decls_ret.remaining);
    try {
        res.sim_cmds = parse_vcd_sim_cmds(decls_ret.remaining, path);
    } catch (const VCDDeclParseError &decl_parse_error) {
        fmt::print(stderr, "Error parsing VCD simulation commands:\n{:s}\n",
                   decl_parse_error.what());
    }

    auto doc_res =
        lexy::parse<grammar::vcd_document>(input, lexy_ext::report_error.path(path.c_str()));
    fmt::print("doc success: {}\n", doc_res.is_success());
    if (doc_res.is_success()) {
        res = std::move(doc_res.value());
    }

    fmt::print("decls: {}\n", fmt::join(res.declarations, ", "));
    fmt::print("cmds: {}\n", fmt::join(res.sim_cmds, ", "));
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
