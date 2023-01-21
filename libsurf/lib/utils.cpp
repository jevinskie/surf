#include "utils.h"

#include <sys/sysctl.h>

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
#ifdef __APPLE__
    uint32_t num;
    size_t sz = sizeof(num);
    posix_check(sysctlbyname("hw.logicalcpu", &num, &sz, nullptr, 0), "get_num_cores");
    return num;
#else
#error get_num_cores unimplemented OS
#endif
}

}; // namespace surf
