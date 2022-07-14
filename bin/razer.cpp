#include "razer.h"
#include "razer_impl.h"
using namespace mbgl;

Razer::Razer(const std::string& style_path)
    : impl(std::make_unique<RazerImpl>(style_path)) {
}

std::shared_ptr<image32_t> Razer::render_tile(long l, long r, long c) {
    return impl->render_tile(l, r, c);
}