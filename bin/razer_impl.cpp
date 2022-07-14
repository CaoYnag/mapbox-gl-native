#include <mbgl/map/map_options.hpp>
#include <mbgl/util/projection.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/image.hpp>
#include <stdexcept>
#include "razer_impl.h"
#include "razer.h"
using namespace mbgl;

std::string read_file(const std::string& path){
    std::string ret;
    FILE* fp = fopen(path.c_str(), "r");
    const int batch = 1024;
    char buff[batch];
    int n = batch;
    while(n >= batch){
        n = fread(buff, 1, batch, fp);
        ret += std::string(buff, n);
    }
    fclose(fp);
    return ret;
}

RazerImpl::RazerImpl(const std::string& style_path)
    : frontend({TILE_SIZE, TILE_SIZE}, 1.0),
      map(frontend, MapObserver::nullObserver(),
          MapOptions().withMapMode(MapMode::Tile).withSize({TILE_SIZE, TILE_SIZE}).withPixelRatio(1.f),
          ResourceOptions()) {
    const auto& style_json = read_file(style_path);
    if(style_json.empty()) throw std::runtime_error("error load style json.");
    map.getStyle().loadJSON(style_json);
}

std::shared_ptr<image32_t> RazerImpl::render_tile(long l, long r, long c) {
    const auto& ll = Projection::unproject({c + .5, r + .5}, pow(2, l) / util::tileSize);
    map.jumpTo(CameraOptions().withCenter(ll).withZoom(l));
    PremultipliedImage pim =  frontend.render(map).image;
    auto im = std::make_shared<image32_t>();
    im->width = pim.size.width;
    im->height = pim.size.height;
    size_t sz = im->width * im->height * image32_t::CHANNELS;
    im->data = std::make_shared<uint8_t[]>(sz);
    uint8_t* src_data = pin.data.get();
    std::copy(src_data, src_data + sz, im->data);
    return im;
}