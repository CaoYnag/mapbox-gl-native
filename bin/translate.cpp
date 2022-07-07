#include <cmath>
#include <cstdint>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/default_styles.hpp>
#include <mbgl/util/projection.hpp>

#include <mbgl/gfx/backend.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/style/style.hpp>

#include <args.hxx>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>


int translate(long z, long x, long y)
{
    using namespace mbgl;

    auto ll = Projection::unproject({x + .5, y + .5}, std::pow(2, z) / util::tileSize);
    printf("got pos(%.2f, %.2f) for tile(%ld, %ld, %ld)\n", ll.longitude(), ll.latitude(), z, y, x);
    return 0;
}

int main(int argc, char *argv[]) {
    args::ArgumentParser argumentParser("Translate Tool");
    args::HelpFlag helpFlag(argumentParser, "help", "Display this help menu", {"help"});

    args::ValueFlag<long> zValue(argumentParser, "n", "Zoom level", {'z', "zoom"});
    args::ValueFlag<long> xValue(argumentParser, "n", "tile column", {'x', "column"});
    args::ValueFlag<long> yValue(argumentParser, "n", "tile row", {'y', "row"});

    try {
        argumentParser.ParseCLI(argc, argv);
    } catch (const args::Help&) {
        std::cout << argumentParser;
        exit(0);
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << argumentParser;
        exit(1);
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << argumentParser;
        exit(2);
    }

    const long y = yValue ? args::get(yValue) : 0;
    const long x = xValue ? args::get(xValue) : 0;
    const long z = zValue ? args::get(zValue) : 0;

    return translate(z, x, y);
}
