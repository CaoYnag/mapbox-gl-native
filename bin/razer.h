#pragma once
#include <string>
#include <memory>

const int TILE_SIZE = 512;

struct image32_t
{
    const static int CHANNELS = 4;
    uint32_t width, height;
    std::shared_ptr<uint8_t[]> data;
};

class RazerImpl;
class Razer{
public:
    Razer(const std::string& style_path);
    ~Razer() = default;

    std::shared_ptr<image32_t> render_tile(long l, long r, long c);
private:
    std::unique_ptr<RazerImpl> impl;
};