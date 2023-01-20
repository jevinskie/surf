#include "utils.h"

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

}; // namespace surf
