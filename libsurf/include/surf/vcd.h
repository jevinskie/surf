#pragma once

#include "common.h"
#include "mmap.h"
#include "time.h"
#include "trace.h"

namespace surf {

namespace VCDTypes {

class Comment : public std::string {};

struct Declarations {
    std::optional<std::string> comment;
    std::optional<std::string> date;
    std::optional<std::string> version;
};

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

using SimCmd = std::variant<Comment, SimTime, Change>;

struct Document {
    Declarations declarations;
    std::vector<SimCmd> sim_cmds;
};

}; // namespace VCDTypes

class SURF_EXPORT VCDFile {
public:
    VCDFile(const std::filesystem::path &path);
    std::shared_ptr<Trace> surf_trace() const;
    int timebase_power() const;
    Time start() const;
    Time end() const;
    const char *data() const;
    size_t size() const;
    const VCDTypes::Document &document();
    const VCDTypes::Declarations &declarations() const;
    const std::vector<VCDTypes::SimCmd> &sim_cmds();

private:
    void parse_declarations();
    void parse_changes();

    VCDTypes::Document m_document;
    const char *m_sim_cmds;
    bool m_parsed_changes;
    MappedReadOnlyFile m_mapped_file;
    Time m_start;
    Time m_end;
    std::shared_ptr<Trace> m_trace;
};

}; // namespace surf
