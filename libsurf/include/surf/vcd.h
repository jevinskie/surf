#pragma once

#include "common.h"
#include "mmap.h"
#include "time.h"
#include "trace.h"

#include <fmt/format.h>

// #define SURF_TOY

namespace surf {

namespace VCDTypes {

struct Comment {
    std::string comment;
};

struct Date {
    std::string date;
};

struct Version {
    std::string version;
};

struct TimeUnit {
    std::string time_unit;
};

struct Var {
    std::string var;
};

struct Scope {
    std::string scope;
};

struct Timescale {
    int time_number;
    TimeUnit time_unit;
};

#if 0
struct Declarations {
    std::optional<std::vector<Comment>> comments;
    std::optional<Date> date;
    std::optional<Version> version;
    std::optional<Timescale> timescale;
    std::optional<std::vector<Var>> vars;
    std::optional<std::vector<Scope>> scopes;
};
#else
using Declarations = std::vector<int>;
#endif

struct Tick {
    uint64_t tick;
};

struct ID {
    std::string id;
};

struct ScalarValue {
    bool value;
    bool x;
    bool z;
};

using BinaryNum   = uint64_t;
using RealNum     = double;
using VectorValue = std::variant<BinaryNum, RealNum>;
using Value       = std::variant<ScalarValue, BinaryNum, RealNum>;

struct Change {
    Value value;
    ID id;
};

#ifndef SURF_TOY
using SimCmd = std::variant<Comment, Tick, Change>;
#else
using SimCmd       = std::string;
#endif

struct Document {
    Declarations declarations;
    std::vector<SimCmd> sim_cmds;
};

}; // namespace VCDTypes

class SURF_EXPORT VCDFile {
public:
    VCDFile(const std::filesystem::path &path);
    const std::filesystem::path &path() const;
    std::shared_ptr<Trace> surf_trace() const;
    int timebase_power() const;
    Time start() const;
    Time end() const;
    const char *data() const;
    size_t size() const;
    std::string_view string_view() const;
    const VCDTypes::Document &document();
    const VCDTypes::Declarations &declarations() const;
    const std::vector<VCDTypes::SimCmd> &sim_cmds();
    void parse_test() const;

private:
    void parse_declarations();
    void parse_changes();

    VCDTypes::Document m_document;
    std::string_view m_sim_cmds_str;
    bool m_parsed_changes;
    MappedReadOnlyFile m_mapped_file;
    Time m_start;
    Time m_end;
    std::shared_ptr<Trace> m_trace;
};

}; // namespace surf

template <> struct fmt::formatter<surf::VCDTypes::Comment> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::Comment const &comment, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<Comment '{:s}'>", comment.comment);
    }
};

template <> struct fmt::formatter<surf::VCDTypes::Tick> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::Tick const &tick, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<Tick #{:d}>", tick.tick);
    }
};

template <> struct fmt::formatter<surf::VCDTypes::ID> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::ID const &id, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<ID {}>", id.id);
    }
};

template <> struct fmt::formatter<surf::VCDTypes::ScalarValue> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::ScalarValue const &sv, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<ScalarValue V: {} X: {} Z: {}>", sv.value,
                              surf::boolmoji(sv.x), surf::boolmoji(sv.z));
    }
};

template <> struct fmt::formatter<surf::VCDTypes::Change> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::Change const &change, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<Change ID: '{:s}' V: {}>", change.id, change.value);
    }
};
