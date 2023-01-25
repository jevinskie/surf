#include "vcd-parser.h"
#include "common-internal.h"

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

struct decl_list {
    static constexpr auto rule = [] {
        // auto num = dsl::token(dsl::integer<int>);
        // auto num = dsl::p<decimal_number>;
        // return dsl::list(num, dsl::sep(ws));
        // auto num = dsl::sign + dsl::integer<int64_t>;
        // return dsl::list(num, dsl::sep(ws));
        // auto list = dsl::list(num);`
        // auto list = dsl::list(num, dsl::sep(dsl::ascii::space >> ws);
        // return list;
        auto integer = dsl::integer<int>;
        // auto integer = dsl::terminator(dsl::ascii::space).opt_list(dsl::integer<int>);
        return dsl::opt(dsl::list(integer));
    }();

    static constexpr auto bp   = false;
    static constexpr auto sink = false;

    static constexpr auto value = lexy::as_list<std::vector<int>>;
};

struct document {
    // static constexpr auto rule = dsl::p<decimal_number> + dsl::eof;

    static constexpr auto rule = [] {
        // auto num = dsl::p<decimal_number>;
        auto decls = dsl::p<decl_list> + dsl::eof;
        return decls;
    }();

    // static constexpr auto value = lexy::callback<Document>([](std::vector<int> decls) {
    //     Document doc;
    //     doc.num = decls[0];
    //     return doc;
    // });
    static constexpr auto value = lexy::as_list<std::vector<int>> >>
                                  lexy::callback<Document>([](std::vector<int> &&vec) {
                                      return Document{.nums = std::move(vec)};
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
        auto doc = doc_res.value();
        fmt::print("doc: num: {:d} nums: {}\n", doc.num, fmt::join(doc.nums, ", "));
    } else {
        fmt::print("doc: no value!\n");
    }
}

}; // namespace surf
