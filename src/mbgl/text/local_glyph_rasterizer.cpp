#include <GL/gl.h>
#include <algorithm>
#include <cstdint>
#include <mbgl/text/local_glyph_rasterizer.hpp>
#include <mapbox/glyph_foundry.hpp>
#include <mapbox/glyph_foundry_impl.hpp>
#include <map>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include <tuple>
#include "mbgl/text/glyph.hpp"
#include "mbgl/util/font_stack.hpp"
#include "mbgl/util/image.hpp"
#include <mbgl/util/tiny_sdf.hpp>
namespace fs = std::filesystem;
namespace sgf = sdf_glyph_foundry;
namespace mbgl {

struct FaceGuard{
    FaceGuard(FT_Face _face) : face(_face){}
    ~FaceGuard(){
        if(face) FT_Done_Face(face);
    }
    FT_Face face;
};

struct LibGuard{
    LibGuard(FT_Library _lib) : lib(_lib) {}
    ~LibGuard(){
        if(lib) FT_Done_FreeType(lib);
    }
    FT_Library lib;
};

class LocalGlyphRasterizer::Impl {
public:
    // load all fonts in path
    Impl(const std::string& path){
        FT_Library lib = nullptr;
        FT_Error err = FT_Init_FreeType(&lib);
        if(err){
            // log error here
            throw std::runtime_error("failed init freetype library.");
        }
        libg = std::make_unique<LibGuard>(lib);
        loadFace(path);
    }
    ~Impl() = default;
    bool canRasterizeGlyph(const FontStack& fs, GlyphID id){
        return std::any_of(fs.begin(), fs.end(), [&](const std::string& name){
            if(faces.count(name)){
                FT_Face face = faces[name]->face;
                return FT_Get_Char_Index(face, id) > 0;
            }
            return false;
        });
    }
    Glyph rasterizeGlyph(const FontStack& fs, GlyphID id){
        for(const auto& font : fs){
            if(faces.count(font)){
                FT_Face face = faces[font]->face;
                FT_UInt idx = FT_Get_Char_Index(face, id);
                if(!idx) continue;

                /*
                auto glyph = std::make_shared<sgf::glyph_info>();
                glyph->char_index = idx;
                sgf::RenderSDF(*glyph, 30, Glyph::borderSize, 0.25, face);
                auto ext = Glyph::borderSize * 2;
                printf("success render[0x%x] with font[%s]: ori(%d, %d) sz(%d, %d) im(%d, %d) advance %d\n", 
                    id, font.c_str(), glyph->left, glyph->top, glyph->width, glyph->height,
                    glyph->width + ext, glyph->height + ext,  static_cast<uint32_t>(glyph->advance));

                return {id, 
                AlphaImage({glyph->width + ext, glyph->height + ext}, reinterpret_cast<const uint8_t*>(glyph->bitmap.c_str()), glyph->bitmap.length()), 
                {
                    glyph->width,
                    glyph->height,
                    glyph->left,
                    glyph->top,
                    static_cast<uint32_t>(glyph->advance)
                }};
                */
                ///*
                FT_Error err = FT_Load_Glyph(face, idx, FT_LOAD_DEFAULT);
                if(err){
                    //printf("failed load[0x%x] with font[%s]: 0x%x - %s\n", id, font.c_str(), err, FT_Error_String(err));
                    continue;
                }
                err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
                if(err) {
                    //printf("failed render[0x%x] with font[%s]: 0x%x - %s\n", id, font.c_str(), err, FT_Error_String(err));
                    continue;
                }
                //printf("success render[0x%x] with font[%s]\n", id, font.c_str());

                auto* glyph = face->glyph;
                uint32_t wid = glyph->bitmap.width;
                uint32_t hgt = glyph->bitmap.rows;
                Glyph local{id, AlphaImage({wid, hgt}, glyph->bitmap.buffer, static_cast<unsigned long>(wid) * hgt),
                {
                    wid - 6, hgt - 6,
                    0, 0,
                    static_cast<uint32_t>(glyph->metrics.horiAdvance >> 6)
                }};
                //local.bitmap = util::transformRasterToSDF(local.bitmap, 8, .25); // origin bak
                local.bitmap = util::transformRasterToSDF(local.bitmap, 2, .25);
                return local;
                //*/
            }
        }
        return {};
    }
    void updateFontPath(const std::string& path){
        loadFace(path);
    }
private:
    /* try load a new face, only used by canRasterizeGlyph */
    void loadFace(const fs::path& path){
        if(!fs::is_directory(path)) return;
        for(const auto& item : fs::directory_iterator{path}){
            if(fs::is_regular_file(item))
                loadFile(item.path());
        }
    }
    void loadFile(const fs::path& path){
        FT_Library lib = libg->lib;
        FT_Face test = nullptr;
        FT_Error err = FT_New_Face(lib, path.c_str(), -1, &test);
        if(err) return; // not a valid font file

        char buff[256];
        for(int idx = 0; idx < test->num_faces; ++idx){
            FT_Face face = nullptr;
            err = FT_New_Face(lib, path.c_str(), idx, &face);
            if(err) {
                // TODO log and continue
                continue;
            }
            // TODO in debug mode, show detail.
            sprintf(buff, "%s %s", face->family_name, face->style_name);
            printf("loaded local font %s in file %s\n", buff, path.c_str());
            constexpr const long size = 30;
            FT_Set_Char_Size(face, 0, size << 6, 0, 0);
            //err = FT_Set_Pixel_Sizes(face, 30, 0); // wid, hgt
            faces.emplace(buff, std::make_shared<FaceGuard>(face));
        }
        FT_Done_Face(test);
    }

    std::unique_ptr<LibGuard> libg;
    std::map<std::string, std::shared_ptr<FaceGuard>> faces;
};

LocalGlyphRasterizer::LocalGlyphRasterizer(const optional<std::string>& /*path*/) : impl(std::make_unique<Impl>("fonts")) {
}

LocalGlyphRasterizer::~LocalGlyphRasterizer() = default;

bool LocalGlyphRasterizer::canRasterizeGlyph(const FontStack& fs, GlyphID id) {
    return impl->canRasterizeGlyph(fs, id);
}

Glyph LocalGlyphRasterizer::rasterizeGlyph(const FontStack& fs, GlyphID id) {
    return impl->rasterizeGlyph(fs, id);
}

void LocalGlyphRasterizer::updateFontPath(const std::string& path){
    impl->updateFontPath(path);
}

} // namespace mbgl
