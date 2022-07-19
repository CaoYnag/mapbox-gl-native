#pragma once
#include <string>
#include <memory>

const int TILE_SIZE = 512;

struct image32_t
{
    const static int CHANNELS = 4;
    uint32_t width, height;
    size_t size;
    std::unique_ptr<uint8_t[]> data;

    image32_t(uint32_t w, uint32_t h);
};

class Razer {
public:
    virtual ~Razer() = default;
    virtual std::shared_ptr<image32_t> render_tile(long l, long r, long c) = 0;
    virtual std::string render_png(long l, long r, long c) = 0;

    static std::shared_ptr<Razer> __attribute__((visibility("default"))) GetRazer(const std::string& style_path);
};

