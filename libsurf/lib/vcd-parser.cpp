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
        return dsl::terminator(dsl::token(LEXY_LIT("$enddefinitions") + (dsl::peek(ws) >> ws) +
                                          LEXY_LIT("$end")))
            .list(num);
    }();

    static constexpr auto value = lexy::as_list<std::vector<int>>;
};

struct sim_cmd_list {
    static constexpr auto rule  = dsl::list(dsl::peek_not(dsl::eof) >> dsl::p<word>);
    static constexpr auto value = lexy::as_list<std::vector<std::string>>;
};

struct decls_and_cmds {
    static constexpr auto rule = dsl::p<decl_list> + dsl::p<sim_cmd_list> + dsl::eof;
    static constexpr auto value =
        lexy::callback<Document>([](std::vector<int> &&decls, std::vector<std::string> &&sim_cmds) {
            return Document{.nums = std::move(decls), .words = std::move(sim_cmds)};
        });
};

struct just_decls {
    static constexpr auto rule  = dsl::p<decl_list> + dsl::eof;
    static constexpr auto value = lexy::callback<Document>([](std::vector<int> &&decls) {
        return Document{.nums = std::move(decls)};
    });
};

struct just_cmds {
    static constexpr auto rule  = dsl::p<sim_cmd_list> + dsl::eof;
    static constexpr auto value = lexy::callback<Document>([](std::vector<std::string> &&sim_cmds) {
        return Document{.words = std::move(sim_cmds)};
    });
};

struct empty {
    static constexpr auto rule  = dsl::eof;
    static constexpr auto value = lexy::callback<Document>([]() {
        return Document{};
    });
};

struct document {
    static constexpr auto ds_and_cs = dsl::p<decls_and_cmds>;
    static constexpr auto ds        = dsl::p<just_decls>;
    static constexpr auto cs        = dsl::p<just_cmds>;
    static constexpr auto ef        = dsl::p<empty>;

#if 0    
    static constexpr auto a = dsl::else_ >> dsl::if_(dsl::peek(decls_and_cmds) >> decls_and_cmds);
    static constexpr auto b = a | dsl::else_ >> dsl::if_(dsl::peek(just_decls) >> just_decls);
    static constexpr auto c = b | dsl::else_ >> dsl::if_(dsl::peek(just_cmds) >> just_cmds);
    static constexpr auto d = c | dsl::else_ >> empty;
    static constexpr auto d = c + dsl::if_(dsl::peek(dsl::eof));
    static constexpr auto d = c | empty;
    static constexpr auto c = b | empty;
    static constexpr auto a = dsl::else_ >> decls_and_cmds;
    static constexpr auto b = a | dsl::else_ >> just_decls;
    static constexpr auto c = b | dsl::else_ >> just_cmds;
    static constexpr auto d = c | dsl::else_ >> empty;
#endif

#if 0
    static constexpr auto a = dsl::peek(decls_and_cmds) >> decls_and_cmds;
    static constexpr auto b = a | dsl::peek(just_decls) >> just_decls;
    static constexpr auto c = b | dsl::peek(just_cmds) >> just_cmds;
    static constexpr auto d = c | dsl::else_ >> empty;
    static constexpr auto rule = d;
#endif

#if 1
    static constexpr auto a    = dsl::peek(ds_and_cs);
    static constexpr auto b    = a | dsl::else_ >> dsl::try_(ds);
    static constexpr auto c    = b | dsl::else_ >> dsl::try_(cs);
    static constexpr auto d    = c | dsl::else_ >> ef;
    static constexpr auto rule = d;
#endif

    // static constexpr auto rule = dsl::try_(decls) + dsl::try_(cmds) + dsl::eof;
    // static constexpr auto rule = dsl::peek(decls_and_cmds) >> decls_and_cmds | dsl::else_ >>
    // just_decls | dsl::else_ >> just_cmds | dsl::else_ >> empty;

    // static constexpr auto value = lexy::callback<Document>();

    static constexpr auto value = lexy::callback<Document>(
        [](Document &&doc) {
            return std::move(doc);
        },
        []() {
            fmt::print("empty wtf\n");
            return Document{};
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
