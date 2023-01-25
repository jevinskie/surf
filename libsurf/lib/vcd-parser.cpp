#include "vcd-parser.h"
#include "common-internal.h"
#include <utils.h>

#include <boost/spirit/home/x3.hpp>

#include <lexy/action/parse.hpp>         // lexy::parse
#include <lexy/action/parse_as_tree.hpp> // lexy::parse_as_tree
#include <lexy/action/trace.hpp>         // lexy::trace_to
#include <lexy/callback.hpp>             // value callbacks
#include <lexy/dsl.hpp>                  // lexy::dsl::*
#include <lexy/input/string_input.hpp>   // lexy::string_input
#include <lexy/parse_tree.hpp>           // lexy::parse_tree_for

#include <lexy_ext/report_error.hpp> // lexy_ext::report_error

#include <concurrencpp/concurrencpp.h>

using namespace VCDTypes;

namespace surf {

namespace {

namespace grammar {
namespace dsl = lexy::dsl;

static constexpr auto ws = dsl::whitespace(dsl::ascii::space);

struct decimal_number {
    static constexpr auto rule  = dsl::sign + dsl::integer<int>;
    static constexpr auto value = lexy::as_integer<int>;
};

struct word {
    static constexpr auto rule  = dsl::identifier(dsl::ascii::word);
    static constexpr auto value = lexy::as_string<std::string>;
};

struct sim_cmd {
    static constexpr auto rule  = dsl::p<word>;
    static constexpr auto value = lexy::forward<std::string>;
};

struct decl_list {
    static constexpr auto rule = [] {
        auto num = dsl::p<decimal_number>;
        return dsl::terminator(LEXY_LIT("$enddefinitions")).list(num) + LEXY_LIT("$end");
    }();

    static constexpr auto value = lexy::as_list<std::vector<int>>;
};

struct sim_cmd_list {
    static constexpr auto rule  = dsl::list(dsl::peek_not(dsl::eof) >> dsl::p<word>);
    static constexpr auto value = lexy::as_list<std::vector<std::string>>;
};

struct document {
    static constexpr auto rule =
        dsl::try_(dsl::p<decl_list>) + dsl::try_(dsl::p<sim_cmd_list>) + dsl::eof;

    static constexpr auto value = lexy::callback<Document>(
        [](std::optional<std::vector<int>> &&decls) {
            return Document{.nums = std::move(decls), .words = {}};
        },
        [](std::vector<int> &&decls, std::vector<std::string> &&sim_cmds) {
            return Document{.nums = std::move(decls), .words = std::move(sim_cmds)};
        },
        [](std::vector<std::string> &&sim_cmds) {
            return Document{.nums = {}, .words = std::move(sim_cmds)};
        },
        []() {
            return Document{.nums = {}, .words = {}};
        });

    static constexpr auto whitespace = ws;
};

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
    return {.declarations = std::move(declsret.decls),
            .sim_cmds     = parse_vcd_sim_cmds(declsret.remaining)};
}

void parse_vcd_document_test(std::string_view vcd_str, const fs::path &path) {
    auto input = lexy::string_input<lexy::ascii_encoding>(vcd_str);
    auto validate_res =
        lexy::validate<grammar::document>(input, lexy_ext::report_error.path(path.c_str()));
    fmt::print("is_error: {}\n", validate_res.is_error());

    lexy::parse_tree_for<decltype(input)> parse_tree;
    auto tree_res = lexy::parse_as_tree<grammar::document>(parse_tree, input, lexy::noop);
    if (tree_res.is_success()) {
        lexy::visualize(stdout, parse_tree, {lexy::visualize_fancy});
    } else {
        fmt::print("parse_as_tree error: {}\n", tree_res.errors());
    }

    std::string trace;
    lexy::trace_to<grammar::document>(std::back_insert_iterator(trace), input,
                                      {lexy::visualize_fancy});
    fmt::print("{:s}\n", trace);

    auto doc_res = lexy::parse<grammar::document>(input, lexy_ext::report_error.path(path.c_str()));
    if (doc_res.has_value()) {
        auto doc = std::move(doc_res.value());
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

}; // namespace surf
