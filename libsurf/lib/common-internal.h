#pragma once

#include <surf/common.h>

using namespace surf;

#undef NDEBUG
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <unistd.h>

#include <fmt/format.h>

namespace fs = std::filesystem;
using namespace std::string_literals;
