#include <math.h>
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
#include <chrono>
#include <tuple>
using namespace mbgl;
using lrc_t = std::tuple<long, long, long>;

const int size = 512;

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

long render(HeadlessFrontend& front, Map& map, const lrc_t& lrc, const std::string& output){
    long l = std::get<0>(lrc);
    long r = std::get<1>(lrc);
    long c = std::get<2>(lrc);
    auto ll = Projection::unproject({c + .5, r + .5}, pow(2, l) / util::tileSize);
    auto s = std::chrono::high_resolution_clock::now();
    map.jumpTo(CameraOptions()
                   .withCenter(ll)
                   .withZoom(l));
    try {
        std::ofstream out(output, std::ios::binary);
        auto rslt = front.render(map);
        auto e = std::chrono::high_resolution_clock::now();
        out << encodePNG(rslt.image);
        out.close();
        return std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count();
    } catch(std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
       return 1;
    }
}

int render_main(const std::vector<lrc_t>& lrcs, const std::string& style, const std::string& output)
{
    util::RunLoop loop;
    HeadlessFrontend frontend({ size, size }, 1.0);
    Map map(frontend, MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Tile).withSize(frontend.getSize()).withPixelRatio(1.0),
            ResourceOptions());


    map.getStyle().loadJSON(read_file(style));
    map.setDebug(mbgl::MapDebugOptions::TileBorders | mbgl::MapDebugOptions::ParseStatus);
    /*{
        // remove all exists source
        map.getStyle().removeSources();
        const char* SOURCE_NAME = "source";
        auto source = std::make_unique<mbgl::style::VectorSource>(SOURCE_NAME);
    }*/
    double total = .0;
    for(const auto& lrc : lrcs){
        char buff[256];
        sprintf(buff, "%s_%02ld_%ld_%ld.png", output.c_str(), std::get<0>(lrc), std::get<1>(lrc), std::get<2>(lrc));
        auto c = render(frontend, map, lrc, buff);
        printf("render tile(%ld, %ld, %ld) cost %ldms\n", std::get<0>(lrc), std::get<1>(lrc), std::get<2>(lrc), c);
        total += c;
    }
    if(lrcs.size())
        printf("avg cost: %.2lf\n", total / lrcs.size());
    return 0;
}

int main(int argc, char *argv[]) {
    args::ArgumentParser argumentParser("Rasterize Tool");
    args::HelpFlag helpFlag(argumentParser, "help", "Display this help menu", {"help"});

    args::ValueFlag<std::string> styleValue(argumentParser, "file", "Map stylesheet", {'s', "style"});
    args::ValueFlag<std::string> outputValue(argumentParser, "file", "Output file name/fmt", {'o', "output"});

    args::ValueFlag<std::string> tileValue(argumentParser, "n", "Tile LRC", {'t', "tile"});
    args::ValueFlag<double> lonValue(argumentParser, "degree", "longitude", {'x', "lon"});
    args::ValueFlag<double> latValue(argumentParser, "degree", "latitude", {'y', "lat"});
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

    std::vector<lrc_t> lrcs;
    if(tileValue) {
        const std::string tile = args::get(tileValue);
        long l, r, c;
        sscanf(tile.c_str(), "%ld,%ld,%ld", &l, &r, &c);
        printf("adding tile(%ld, %ld, %ld)\n", l, r, c);
        lrcs.emplace_back(l, r, c);
    } else if(lonValue && latValue) {
        // add tiles contains(lat, lon) from zoom_range(0, max)
        const long MAX_ZOOM = 17;
        const double lat = args::get(latValue);
        const double lon = args::get(lonValue);
        for(long z = 0; z < MAX_ZOOM; ++z){
            const double scale = std::pow(2.0, z);
            auto p = Projection::project({lat, lon}, scale) / util::tileSize;
            lrcs.emplace_back(z, p.y, p.x);
        }
    } else{
        std::cerr << "neither tile or (lat, lon) were specified, render(0, 0, 0) for default. "<< std::endl;
        lrcs.emplace_back(0, 0, 0);
    }

    const std::string output = outputValue ? args::get(outputValue) : "out.png";
    std::string style = styleValue ? args::get(styleValue) : "style.json";

    return render_main(lrcs, style, output);
}
