#pragma once
#include <string>
#include <memory>
#include <mbgl/util/image.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/map.hpp>

const int TILE_SIZE = 512;

class Rasterizer{
public:
    Rasterizer(const std::string& style_path);
    ~Rasterizer() = default;

    mbgl::PremultipliedImage render_tile(long l, long r, long c);
private:
    mbgl::util::RunLoop loop;
    mbgl::HeadlessFrontend frontend;
    mbgl::Map map;
};