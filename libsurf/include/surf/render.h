#pragma once

#include "common.h"
#include "time.h"
#include "trace.h"

namespace surf {

class SURF_EXPORT Renderer {
public:
    Renderer(const Trace &trace);
    void render(const Time &start, const Time &end, uint32_t width, uint32_t height,
                const std::filesystem::path &png_path);

private:
    const Trace &m_trace;
};

} // namespace surf
