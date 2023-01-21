#include <surf/mmap.h>

#include "common-internal.h"
#include "utils.h"

#include <sys/mman.h>
#include <sys/stat.h>

MappedReadOnlyFile::MappedReadOnlyFile(const fs::path &path, const void *preferred_addr) {
    const auto fd_res = open(path.c_str(), O_RDONLY);
    posix_check(fd_res, "MappedReadOnlyFile open");

    struct stat st {};
    const auto fstat_res = fstat(fd_res, &st);
    posix_check(fstat_res, "MappedReadOnlyFile fstat");

    const auto *buf =
        (const uint8_t *)mmap((void *)preferred_addr, st.st_size, PROT_READ,
                              MAP_PRIVATE | (preferred_addr ? MAP_FIXED : 0), fd_res, 0);
    if (buf == MAP_FAILED) {
        throw std::system_error(std::make_error_code((std::errc)errno), "MappedReadOnlyFile mmap");
    }
    const auto close_res = close(fd_res);
    posix_check(close_res, "MappedReadOnlyFile close");

    m_mapping = buf;
    m_size    = st.st_size;
}

MappedReadOnlyFile::~MappedReadOnlyFile() {
    const auto munmap_res = munmap((void *)m_mapping, m_size);
    posix_check(munmap_res, "MappedReadOnlyFile munmap");
}

const uint8_t *MappedReadOnlyFile::data() const {
    return m_mapping;
}

size_t MappedReadOnlyFile::size() const {
    return m_size;
}