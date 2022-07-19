#include "razer.h"
#include <mbgl/map/map_options.hpp>
#include <mbgl/util/projection.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/map.hpp>
#include <stdexcept>

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

image32_t::image32_t(uint32_t w, uint32_t h)
    : width(w), height(h), size(w * h * image32_t::CHANNELS),
      data(std::make_unique<uint8_t[]>(size))
{}

class RazerImpl : public Razer {
public:
    RazerImpl(const std::string& style_path)
    : frontend({TILE_SIZE, TILE_SIZE}, 1.0),
      map(frontend, mbgl::MapObserver::nullObserver(),
          mbgl::MapOptions().withMapMode(mbgl::MapMode::Tile).withSize({TILE_SIZE, TILE_SIZE}).withPixelRatio(1.f),
          mbgl::ResourceOptions()) {
        const auto& style_json = read_file(style_path);
        if(style_json.empty()) throw std::runtime_error("error load style json.");
        map.getStyle().loadJSON(style_json);
    }

    ~RazerImpl() = default;

    std::shared_ptr<image32_t> render_tile(long l, long r, long c) {
        try{
            const auto& ll = mbgl::Projection::unproject({c + .5, r + .5}, pow(2, l) / mbgl::util::tileSize);
            map.jumpTo(mbgl::CameraOptions().withCenter(ll).withZoom(l));
            auto pim =  frontend.render(map).image;
            auto im = std::make_shared<image32_t>(pim.size.width, pim.size.height);
            auto src_data = pim.data.get();
            std::copy(src_data, src_data + pim.bytes(), im->data.get());
            return im;
        } catch(std::exception& e){
            return nullptr;
        }
    }

    std::string render_png(long l, long r, long c) {
        try{
            const auto& ll = mbgl::Projection::unproject({c + .5, r + .5}, pow(2, l) / mbgl::util::tileSize);
            map.jumpTo(mbgl::CameraOptions().withCenter(ll).withZoom(l));
            return encodePNG(frontend.render(map).image);
        } catch(std::exception& e){
            return std::string();
        }
    }
private:
    mbgl::util::RunLoop loop;
    mbgl::HeadlessFrontend frontend;
    mbgl::Map map;
};

std::shared_ptr<Razer> Razer::GetRazer(const std::string& style_path) {
    return std::make_shared<RazerImpl>(style_path);
}