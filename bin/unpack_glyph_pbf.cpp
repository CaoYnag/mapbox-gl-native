#include "mbgl/text/glyph_pbf.hpp"
#include <iostream>
#include <filesystem>
#include <string>
using namespace std;
namespace fs = std::filesystem;

std::string read_file(const string& path)
{
    int sz = fs::file_size(path);
    char buff[sz];
    FILE* fp = fopen(path.c_str(), "rb");
    fread(buff, 1, sz, fp);
    fclose(fp);
    return string(buff, sz);
}

void write_glyph(const mbgl::Glyph& g){
    char buff[256];
    sprintf(buff, "glyphs/%d_%d_%d_%d_%d_%d_%d_%d",
        g.id, g.metrics.width, g.metrics.height,
        g.metrics.left, g.metrics.top, g.metrics.advance,
        g.bitmap.size.width, g.bitmap.size.height);
    FILE* fp = fopen(buff, "w+");
    fwrite(g.bitmap.data.get(), 1, g.bitmap.size.width * g.bitmap.size.height, fp);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if(argc < 2) return 1;
    auto glyphs = mbgl::parseGlyphPBF(mbgl::GlyphRange(0, 9999999), read_file(argv[1]));
    for(const auto& glyph : glyphs){
        write_glyph(glyph);
    }
    return 0;
}
