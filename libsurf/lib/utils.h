#pragma once

#include "common-internal.h"

#include <bit>

#if 0
#include <backward.hpp>

#define DUMP_STACK(name)                                                                           \
    do {                                                                                           \
        {                                                                                          \
            std::cout << (name) << " STACK BEGIN:" << std::endl;                                   \
            backward::StackTrace st;                                                               \
            st.load_here(32);                                                                      \
            backward::Printer pr;                                                                  \
            pr.snippet    = true;                                                                  \
            pr.color_mode = backward::ColorMode::always;                                           \
            pr.address    = true;                                                                  \
            pr.object     = true;                                                                  \
            pr.print(st);                                                                          \
            std::cout << (name) << " STACK END" << std::endl;                                      \
        }                                                                                          \
    } while (0)
#define MCA_BEGIN(name)                                                                            \
    do {                                                                                           \
        __asm volatile("# LLVM-MCA-BEGIN " #name ::: "memory");                                    \
    } while (0)
#define MCA_END(name)                                                                              \
    do {                                                                                           \
        __asm volatile("# LLVM-MCA-END " #name ::: "memory");                                      \
    } while (0)
#endif

namespace surf {

void posix_check(int retval, const std::string &msg);

unsigned int get_num_cores();

bool can_use_term_colors();

// for visit(overload(...case lambdas...), variant_var)
template <typename... T> class overload : T... {
public:
    overload(T... t) : T(t)... {}
    using T::operator()...;
};

// https://www.modernescpp.com/index.php/c-20-pythons-map-function
template <typename Func, typename Seq> auto map(Func func, Seq seq) {
    typedef typename Seq::value_type value_type;
    using return_type = decltype(func(std::declval<value_type>()));

    std::vector<return_type> result{};
    for (auto &i : seq) {
        result.emplace_back(func(i));
    }

    return result;
}

}; // namespace surf
