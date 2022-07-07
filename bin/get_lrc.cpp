#include <mbgl/util/tile_cover.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/map/transform.hpp>
#include <args.hxx>
#include <iostream>
using namespace mbgl;

void get_lrc(double lat, double lon, double zoom, uint32_t size){
    Transform transform;
    transform.resize({ size, size });
    // slightly offset center so that tile order is better defined

    transform.jumpTo(CameraOptions().withCenter(LatLng { lat, lon }).withZoom(zoom));

    auto rslt =  util::tileCover(transform.getState(), zoom);
    for(const auto& tile : rslt){
        printf("(%d, %d, %d)\n", tile.canonical.z, tile.canonical.y, tile.canonical.x);
    }
}

int main(int argc, char *argv[]) {
    args::ArgumentParser argumentParser("Get LRC with specified parameters");
    args::HelpFlag helpFlag(argumentParser, "help", "Display this help menu", {"help"});

    args::ValueFlag<double> zoomValue(argumentParser, "number", "Zoom level", {'z', "zoom"});
    args::ValueFlag<double> lonValue(argumentParser, "degrees", "Longitude", {'x', "lon"});
    args::ValueFlag<double> latValue(argumentParser, "degrees", "Latitude", {'y', "lat"});
    args::ValueFlag<uint32_t> sizeValue(argumentParser, "pixels", "Image Size", {'S', "size"});

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

    const double lat = latValue ? args::get(latValue) : 0;
    const double lon = lonValue ? args::get(lonValue) : 0;
    const double zoom = zoomValue ? args::get(zoomValue) : 0;
    const uint32_t size = sizeValue ? args::get(sizeValue) : 512;

    get_lrc(lat, lon, zoom, size);
    return 0;
}
