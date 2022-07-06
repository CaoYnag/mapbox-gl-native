#include <mbgl/text/glyph_atlas.hpp>

#include <mapbox/shelf-pack.hpp>

namespace mbgl {

static constexpr uint32_t padding = 1;

GlyphAtlas makeGlyphAtlas(const GlyphMap& glyphs) {
    GlyphAtlas result;

    mapbox::ShelfPack::ShelfPackOptions options;
    options.autoResize = true;
    mapbox::ShelfPack pack(0, 0, options);

    for (const auto& glyphMapEntry : glyphs) {
        FontStackHash fontStack = glyphMapEntry.first;
        GlyphPositionMap& positions = result.positions[fontStack];

        for (const auto& entry : glyphMapEntry.second) {
            if (entry.second && (*entry.second)->bitmap.valid()) {
                const Glyph& glyph = **entry.second;

                const mapbox::Bin& bin = *pack.packOne(-1,
                    glyph.bitmap.size.width + 2 * padding,
                    glyph.bitmap.size.height + 2 * padding);

                result.image.resize({
                    static_cast<uint32_t>(pack.width()),
                    static_cast<uint32_t>(pack.height())
                });

                AlphaImage::copy(glyph.bitmap,
                                 result.image,
                                 { 0, 0 },
                                 {
                                    bin.x + padding,
                                    bin.y + padding
                                 },
                                 glyph.bitmap.size);
                {
                    //! only for debug, write glyph to disk
                    char buff[256];
                    sprintf(buff, "glyphs_cp/%d_%d_%d_%d_%d_%d_%d_%d",
                        glyph.id, glyph.metrics.width, glyph.metrics.height,
                        glyph.metrics.left, glyph.metrics.top, glyph.metrics.advance,
                        result.image.size.width, result.image.size.height);
                    FILE* fp = fopen(buff, "w+");
                    fwrite(result.image.data.get(), 1, result.image.bytes(), fp);
                    fclose(fp);

                    sprintf(buff, "glyphs_ori/%d_%d_%d_%d_%d_%d_%d_%d",
                        glyph.id, glyph.metrics.width, glyph.metrics.height,
                        glyph.metrics.left, glyph.metrics.top, glyph.metrics.advance,
                        glyph.bitmap.size.width, glyph.bitmap.size.height);
                    fp = fopen(buff, "w+");
                    fwrite(glyph.bitmap.data.get(), 1, glyph.bitmap.bytes(), fp);
                    fclose(fp);
                }

                positions.emplace(glyph.id,
                                  GlyphPosition {
                                     Rect<uint16_t> {
                                         static_cast<uint16_t>(bin.x),
                                         static_cast<uint16_t>(bin.y),
                                         static_cast<uint16_t>(bin.w),
                                         static_cast<uint16_t>(bin.h)
                                     },
                                     glyph.metrics
                                  });
            }
        }
        printf("------>DONE GENERATE POSITION MAP: %ld => %ld\n", glyphMapEntry.second.size(), positions.size());
    }

    pack.shrink();
    result.image.resize({
        static_cast<uint32_t>(pack.width()),
        static_cast<uint32_t>(pack.height())
    });

    return result;
}

} // namespace mbgl
