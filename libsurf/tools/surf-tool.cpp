#include <surf/surf.h>
using namespace surf;

#include <string>
#include <unistd.h>

#include <argparse/argparse.hpp>
#include <fmt/format.h>
#include <fmt/std.h>
// #include <fmt/ranges.h>

int main(int argc, const char **argv) {
    argparse::ArgumentParser parser("surf-tool");
    parser.add_argument("-v", "--vcd-trace").help("input VCD file path");
    parser.add_argument("-t", "--surf-trace").help("input Surf file path");
    parser.add_argument("-r", "--render")
        .implicit_value("surf-render.png")
        .help("output PNG render path");
    parser.add_argument("-s", "--start").help("render start time");
    parser.add_argument("-e", "--end").help("render end time");
    parser.add_argument("-T", "--ticks")
        .default_value(false)
        .implicit_value(true)
        .help("use ticks for times");
    parser.add_argument("-W", "--width")
        .default_value((uint32_t)4096)
        .scan<'i', uint32_t>()
        .help("render width (pixels)");
    parser.add_argument("-H", "--height")
        .default_value((uint32_t)4096)
        .scan<'i', uint32_t>()
        .help("render height (pixels)");
    parser.add_argument("-l", "--loop")
        .default_value(false)
        .implicit_value(true)
        .help("loop for testing");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        fmt::print(stderr, "Error parsing arguments: {:s}\n", err.what());
        return -2;
    }

    fmt::print("cwd is: {}\n", std::filesystem::current_path());

    std::shared_ptr<Trace> trace;

    if (const auto vcd_path = parser.present("--vcd-trace")) {
        VCDFile vcd_trace(*vcd_path);
        vcd_trace.parse_test();
        return 0;
        trace = vcd_trace.surf_trace();
    } else {
        if (const auto surf_path = parser.present("--surf-trace")) {
            trace = std::make_shared<Trace>(*surf_path);
        } else {
            fmt::print(stderr, "Missing input trace (VCD or Surf) file.\n");
            return -2;
        }
    }

    Time start_time;
    Time end_time;
    const auto use_ticks = parser["--ticks"] == true;
    const auto width     = parser.get<uint32_t>("--width");
    const auto height    = parser.get<uint32_t>("--height");

    fmt::print("use_ticks: {:b}\n", use_ticks);
    fmt::print("width: {:d} height: {:d}\n", width, height);

    if (parser.get<bool>("--loop")) {
        while (true) {
            usleep(1'000'000);
        }
    }

    assert(trace);

    if (parser.present("--start")) {
        if (use_ticks) {
            start_time = Time(parser.get<uint64_t>("--start"), trace->timebase_power());
        } else {
            start_time = Time(parser.get<double>("--start"), trace->timebase_power());
        }
    } else {
        start_time = trace->start();
    }
    if (parser.present("--end")) {
        if (use_ticks) {
            end_time = Time(parser.get<uint64_t>("--end"), trace->timebase_power());
        } else {
            end_time = Time(parser.get<double>("--end"), trace->timebase_power());
        }
    } else {
        end_time = trace->end();
    }

    if (const auto png_path = parser.present("--render")) {
        Renderer renderer(*trace);
        renderer.render(start_time, end_time, width, height, *png_path);
    }

    fmt::print("hello from surf-tool\n");
    return 0;
}
