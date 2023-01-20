#include <surf/render.h>

#include "common-internal.h"

Renderer::Renderer(const Trace &trace) : m_trace(trace) {}

void Renderer::render(const Time &start, const Time &end, uint32_t width, uint32_t height,
                      const fs::path &png_path) {
    (void)m_trace;
    return;
}
