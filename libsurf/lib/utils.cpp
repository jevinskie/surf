#include "utils.h"

#include <cstdio>

#if defined(SURF_LINUX) || defined(SURF_APPLE)
#include <unistd.h>
#endif
#ifdef SURF_APPLE
#include <sys/sysctl.h>
#endif
#ifdef SURF_WIN
#include <windows.h>
#endif

namespace surf {

void posix_check(int retval, const std::string &msg) {
    if (SURF_UNLIKELY(retval < 0)) {
        const auto errstr =
            fmt::format("POSIX error: '{:s}' retval: {:d} errno: {:d} description: '{:s}'", msg,
                        retval, errno, strerror(errno));
        fmt::print(stderr, "{:s}\n", errstr);
        throw std::system_error(std::make_error_code((std::errc)errno), errstr);
    }
}

uint32_t get_num_cores() {
#if defined(SURF_APPLE)
    uint32_t num;
    size_t sz = sizeof(num);
    posix_check(sysctlbyname("hw.logicalcpu", &num, &sz, nullptr, 0), "get_num_cores");
    return num;
#elif defined(SURF_LINUX)
    return (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
#else
#error get_num_cores unimplemented OS
#endif
}

bool can_use_term_colors() {
#if defined(SURF_LINUX) || defined(SURF_APPLE)
    return isatty(fileno(stdout));
#elif defined(SURF_WIN)
    return _isatty(fileno(stdout));
#endif
}

}; // namespace surf
