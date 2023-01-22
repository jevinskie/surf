#pragma once

#include <surf/common.h>

using namespace surf;

#undef NDEBUG
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <optional>
#include <system_error>
#include <unistd.h>
#include <variant>

#include <fmt/format.h>

namespace fs = std::filesystem;
using namespace std::string_literals;
