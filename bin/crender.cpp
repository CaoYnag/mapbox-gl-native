#include <cstdint>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/default_styles.hpp>

#include <mbgl/gfx/backend.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/style/style.hpp>

#include <args.hxx>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>

std::string read_file(const std::string& path)
{
    std::string ret;
    FILE* fp = fopen(path.c_str(), "r");
    const int batch = 1024;
    char buff[batch];
    int n = batch;
    while(n >= batch)
    {
        n = fread(buff, 1, batch, fp);
        ret += std::string(buff, n);
    }
    
    fclose(fp);
    return ret;
}

int render(long z, double x, double y, uint32_t size, const std::string& style, const std::string& output)
{
    using namespace mbgl;

    util::RunLoop loop;

    HeadlessFrontend frontend({ size, size }, 1.0);
    Map map(frontend, MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()).withPixelRatio(1.0),
            ResourceOptions());


    map.getStyle().loadJSON(read_file(style));
    /*{
        // remove all exists source
        map.getStyle().removeSources();
        const char* SOURCE_NAME = "source";
        auto source = std::make_unique<mbgl::style::VectorSource>(SOURCE_NAME);
    }*/
    map.jumpTo(CameraOptions()
                   .withCenter(LatLng { y, x })
                   .withZoom(z));

    map.setDebug(mbgl::MapDebugOptions::TileBorders | mbgl::MapDebugOptions::ParseStatus);

    try {
        std::ofstream out(output, std::ios::binary);
        std::cout << "before render" << std::endl;
        auto rslt = frontend.render(map);
        std::cout << "end render" << std::endl;
        out << encodePNG(rslt.image);
        out.close();
    } catch(std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
       return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    args::ArgumentParser argumentParser("Rasterize Tool");
    args::HelpFlag helpFlag(argumentParser, "help", "Display this help menu", {"help"});

    args::ValueFlag<std::string> styleValue(argumentParser, "file", "Map stylesheet", {'s', "style"});
    args::ValueFlag<std::string> outputValue(argumentParser, "file", "Output file name", {'o', "output"});
    args::ValueFlag<std::string> sourceValue(argumentParser, "file", "tile json file", {'j', "json"});

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
    const std::string output = outputValue ? args::get(outputValue) : "out.png";
    std::string style = styleValue ? args::get(styleValue) : "style.json";
    std::string tilejson = sourceValue ? args::get(sourceValue) : "tile.json";

    return render(zoom, lon, lat, size, style, output);
}
