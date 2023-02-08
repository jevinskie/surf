// clang++ -std=c++20 -c -o /dev/null -Xclang -fdump-record-layouts libsurf/test/dump-struct.cpp

#include <cstdint>
#include <vector>

struct varbit_heap {
    std::vector<uint8_t> buf;
    bool sign;
};

varbit_heap varbit_heap_dummy;
