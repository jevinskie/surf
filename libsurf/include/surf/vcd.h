#pragma once

#include "common.h"
#include "mmap.h"
#include "time.h"
#include "trace.h"

#define SURF_TOY

namespace surf {

namespace VCDTypes {

using Comment  = std::string;
using Date     = std::string;
using Version  = std::string;
using TimeUnit = std::string;
using Var      = std::string;
using Scope    = std::string;

struct Timescale {
    int time_number;
    TimeUnit time_unit;
};

#ifndef SURF_TOY
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

using SimTime = uint64_t;
using ID      = std::string;

struct ScalarValue {
    bool val;
    bool x;
    bool z;
};

using BinaryNum   = uint64_t;
using RealNum     = double;
using VectorValue = std::variant<BinaryNum, RealNum>;
using Value       = std::variant<ScalarValue, BinaryNum, RealNum>;

struct Change {
    Value val;
    ID id;
};

#ifndef SURF_TOY
using SimCmd = std::variant<Comment, SimTime, Change>;
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
