#include <surf/surf.h>

#include <string>

#include <argparse/argparse.hpp>
#include <fmt/format.h>

int main(int argc, const char **argv) {
    argparse::ArgumentParser parser("surf-tool");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        fmt::print(stderr, "Error parsing arguments: {:s}\n", err.what());
        return -2;
    }

    fmt::print("hello from surf-tool\n");
    return 0;
}
