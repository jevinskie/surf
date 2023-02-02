#pragma once

#include "common.h"
#include "mmap.h"
#include "time.h"
#include "trace.h"

#include <fmt/format.h>
#include <magic_enum.hpp>

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

enum class TimeNumber : uint8_t {
    n1   = 1,
    n10  = 10,
    n100 = 100
};

enum class TimeUnit : int8_t {
    s  = 0,
    ms = -3,
    us = -6,
    ns = -9,
    ps = -12,
    fs = -15
};

enum class VarType : uint8_t {
    event,
    integer,
    parameter,
    real,
    realtime,
    reg,
    supply0,
    supply1,
    time,
    tri,
    triand,
    trior,
    trireg,
    tri0,
    tri1,
    wand,
    wire,
    wor
};

struct Var {
    std::string id;
    int size;
    VarType type;
};

enum class ScopeType : uint8_t {
    begin,
    fork,
    function,
    module,
    task
};

struct Scope {
    std::string id;
    std::vector<Scope> subscopes;
    ScopeType type;
};

struct Timescale {
    TimeNumber time_number;
    TimeUnit time_unit;
};

using Declaration = std::variant<Comment, Date, Version, Timescale, Scope>;

struct Declarations {
    std::optional<std::vector<Comment>> comments;
    std::optional<Date> date;
    std::optional<Version> version;
    std::optional<Timescale> timescale;
    std::optional<std::vector<Var>> vars;
    std::optional<std::vector<Scope>> scopes;
};

struct Tick {
    uint64_t tick;
};

enum class ScalarValueEnum : uint8_t {
    v0 = 0,
    v1 = 1,
    vX = 0b10,
    vZ = 0b100
};

class ScalarValue {
public:
    ScalarValue(char c) {
        if (c == '0') {
            m_sve = ScalarValueEnum::v0;
        } else if (c == '1') {
            m_sve = ScalarValueEnum::v1;
        } else if (c == 'x' || c == 'X') {
            m_sve = ScalarValueEnum::vX;
        } else if (c == 'z' || c == 'Z') {
            m_sve = ScalarValueEnum::vZ;
        } else {
            throw std::out_of_range(
                fmt::format("ScalarValue '{:c}' not recognized as 0, 1, x, X, z, or Z", c));
        }
    }
    ScalarValue(bool b, bool x, bool z) {
        if (SURF_UNLIKELY((int)b + (int)x + (int)z > 1)) {
            throw std::out_of_range(
                fmt::format("ScalarValue: more than one bit set: B: {} X: {} Z: {}", b, x, z));
        }
        if (SURF_LIKELY((int)x + (int)z == 0)) {
            m_sve = b ? ScalarValueEnum::v1 : ScalarValueEnum::v0;
            return;
        } else if (x) {
            m_sve = ScalarValueEnum::vX;
            return;
        } else if (z) {
            m_sve = ScalarValueEnum::vZ;
            return;
        }
    }
    bool b() const {
        return m_sve == ScalarValueEnum::v1;
    }
    bool x() const {
        return m_sve == ScalarValueEnum::vX;
    }
    bool z() const {
        return m_sve == ScalarValueEnum::vZ;
    }

private:
    ScalarValueEnum m_sve;
};

struct BinaryNum {
    uint64_t num;
};

struct RealNum {
    double num;
};

using Value = std::variant<ScalarValue, BinaryNum, RealNum>;

struct Change {
    Value value;
    std::string id;
};

using SimCmd = std::variant<Comment, Tick, Change>;

struct DocumentRawDecls {
    std::vector<Declaration> declarations;
    std::vector<SimCmd> sim_cmds;
};

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

template <> struct fmt::formatter<surf::VCDTypes::BinaryNum> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::BinaryNum const &bnum, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<BinaryNum {:#0b}>", bnum.num);
    }
};

template <> struct fmt::formatter<surf::VCDTypes::RealNum> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::RealNum const &rnum, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<RealNum {}>", rnum.num);
    }
};

template <> struct fmt::formatter<surf::VCDTypes::ScalarValue> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::ScalarValue const &sv, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<ScalarValue V: {} X: {} Z: {}>", sv.b() ? '1' : '0',
                              surf::boolmoji(sv.x()), surf::boolmoji(sv.z()));
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

template <> struct fmt::formatter<surf::VCDTypes::Date> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::Date const &date, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<Date '{:s}'>", date.date);
    }
};

template <> struct fmt::formatter<surf::VCDTypes::Version> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::Version const &version, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<Version '{:s}'>", version.version);
    }
};

template <> struct fmt::formatter<surf::VCDTypes::Timescale> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::Timescale const &timescale, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<Timescale {:d} {:s}>",
                              magic_enum::enum_integer(timescale.time_number),
                              magic_enum::enum_name(timescale.time_unit));
    }
};

template <> struct fmt::formatter<surf::VCDTypes::Scope> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::Scope const &scope, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<Scope {:s} {:s}>", magic_enum::enum_name(scope.type),
                              scope.id);
    }
};

template <> struct fmt::formatter<surf::VCDTypes::Declarations> {
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(surf::VCDTypes::Declarations const &decls, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "<Declarations '{:s}'>", "fmt::TODO");
    }
};
