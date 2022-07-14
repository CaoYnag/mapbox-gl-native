#pragma once
#include <mbgl/util/run_loop.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/map.hpp>
#include <memory>

class image32_t;
class RazerImpl{
public:
    RazerImpl(const std::string& style_path);
    ~RazerImpl() = default;

    std::shared_ptr<image32_t> render_tile(long l, long r, long c);
private:
    mbgl::util::RunLoop loop;
    mbgl::HeadlessFrontend frontend;
    mbgl::Map map;
};