#pragma once

#include "common.h"

namespace surf {

class SURF_EXPORT MappedReadOnlyFile {
public:
    MappedReadOnlyFile(const std::filesystem::path &path, const void *preferred_addr = nullptr);
    ~MappedReadOnlyFile();
    const uint8_t *data() const;
    size_t size() const;
    std::string_view string_view() const;
    const std::filesystem::path &path() const;

private:
    const std::filesystem::path m_path;
    const uint8_t *m_mapping;
    size_t m_size;
};

}; // namespace surf
