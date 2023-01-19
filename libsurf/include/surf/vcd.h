#pragma once

#include "common.h"

#include <filesystem>

namespace surf {

class SURF_EXPORT VCD {
public:
    VCD(const std::filesystem::path &path);

private:
    const std::filesystem::path m_path;
};

} // namespace surf
