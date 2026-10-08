#include "uranium/render/font.hpp"
#include "uranium/core/error.hpp"
#include "../../third_party/stb/stb_truetype.h"
#include <cstdlib>
#include <fstream>
#include <utility>
namespace uranium {
namespace render {
struct Font::Impl {
    std::vector<std::uint8_t> data;
    stbtt_fontinfo info{};
    bool ok = false;
    std::string name;
};

Font::Font() : m_impl(std::make_shared<Impl>()) {}
Font::Font(const Font& other) = default;
Font::Font(Font&& other) noexcept = default;
Font& Font::operator=(const Font& other) = default;
Font& Font::operator=(Font&& other) noexcept = default;
Font::~Font() = default;
bool Font::load_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (bytes.empty()) return false;
    bool ok = load_memory(bytes.data(), bytes.size());
    if (ok) m_impl->name = path;
    return ok;
}

bool Font::load_memory(const std::uint8_t* data, std::size_t size) {
    if (!data || size == 0) return false;
    m_impl->data.assign(data, data + size);
    int offset = stbtt_GetFontOffsetForIndex(m_impl->data.data(), 0);
    m_impl->ok = stbtt_InitFont(&m_impl->info, m_impl->data.data(), offset) != 0;
    return m_impl->ok;
}

bool Font::loaded() const { return m_impl->ok; }
const std::string& Font::name() const { return m_impl->name; }
float Font::ascent(float px) const {
    if (!m_impl->ok) return px * 0.8f;
    int a = 0, d = 0, g = 0;
    stbtt_GetFontVMetrics(&m_impl->info, &a, &d, &g);
    return a * stbtt_ScaleForPixelHeight(&m_impl->info, px);
}

float Font::descent(float px) const {
    if (!m_impl->ok) return px * 0.2f;
    int a = 0, d = 0, g = 0;
    stbtt_GetFontVMetrics(&m_impl->info, &a, &d, &g);
    return -d * stbtt_ScaleForPixelHeight(&m_impl->info, px);
}

float Font::line_height(float px) const {
    if (!m_impl->ok) return px;
    int a = 0, d = 0, g = 0;
    stbtt_GetFontVMetrics(&m_impl->info, &a, &d, &g);
    return (a - d + g) * stbtt_ScaleForPixelHeight(&m_impl->info, px);
}

namespace {
std::vector<int> decode_utf8(const std::string& text) {
    std::vector<int> out;
    out.reserve(text.size());
    std::size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        int cp = c;
        std::size_t len = 1;
        if (c < 0x80) {
            cp = c;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < text.size()) {
            cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(text[i + 1]) & 0x3F);
            len = 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < text.size()) {
            cp = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 6) | (static_cast<unsigned char>(text[i + 2]) & 0x3F);
            len = 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < text.size()) {
            cp = ((c & 0x07) << 18) | ((static_cast<unsigned char>(text[i + 1]) & 0x3F) << 12) | ((static_cast<unsigned char>(text[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(text[i + 3]) & 0x3F);
            len = 4;
        } else {
            cp = 0x3F;
        }
        out.push_back(cp);
        i += len;
    }
    return out;
}
}

float Font::advance(const std::string& text, float px) const {
    if (!m_impl->ok) return static_cast<float>(text.size()) * px * 0.5f;
    float scale = stbtt_ScaleForPixelHeight(&m_impl->info, px);
    float total = 0.0f;
    int prev = 0;
    for (int cp : decode_utf8(text)) {
        int adv = 0, lsb = 0;
        stbtt_GetCodepointHMetrics(&m_impl->info, cp, &adv, &lsb);
        total += adv * scale;
        if (prev != 0) total += stbtt_GetCodepointKernAdvance(&m_impl->info, prev, cp) * scale;
        prev = cp;
    }
    return total;
}

float Font::measure_width(const std::string& text, float px) const { return advance(text, px); }
float Font::kerning(int prev_cp, int next_cp, float px) const {
    if (!m_impl->ok) return 0.0f;
    float scale = stbtt_ScaleForPixelHeight(&m_impl->info, px);
    return stbtt_GetCodepointKernAdvance(&m_impl->info, prev_cp, next_cp) * scale;
}

bool Font::rasterize(int codepoint, float px, GlyphBitmap& out) const {
    out = GlyphBitmap();
    if (!m_impl->ok) return false;
    float scale = stbtt_ScaleForPixelHeight(&m_impl->info, px);
    int cp = codepoint;
    int w = 0, h = 0, xoff = 0, yoff = 0;
    unsigned char* bitmap = stbtt_GetCodepointBitmap(&m_impl->info, scale, scale, cp, &w, &h, &xoff, &yoff);
    int adv = 0, lsb = 0;
    stbtt_GetCodepointHMetrics(&m_impl->info, cp, &adv, &lsb);
    out.width = w;
    out.height = h;
    out.x_offset = xoff;
    out.y_offset = yoff;
    out.advance = adv * scale;
    if (bitmap && w > 0 && h > 0) out.coverage.assign(bitmap, bitmap + static_cast<std::size_t>(w) * h);
    if (bitmap) std::free(bitmap);
    return true;
}

FontBook::FontBook() {
    auto paths = bundled_font_paths();
    for (const auto& p : paths) {
        Font f;
        if (f.load_file(p)) {
            m_fonts.push_back(Entry{"sans", f});
            m_default_name = "sans";
            break;
        }
    }
    std::string dir = paths.empty() ? std::string() : paths.front().substr(0, paths.front().find_last_of('/'));
    if (!dir.empty()) {
        Font bold, italic;
        if (bold.load_file(dir + "/LiberationSans-Bold.ttf")) m_fonts.push_back(Entry{"sans-bold", bold});
        if (italic.load_file(dir + "/LiberationSans-Italic.ttf")) m_fonts.push_back(Entry{"sans-italic", italic});
    }
}

FontBook& FontBook::instance() {
    static FontBook book;
    return book;
}

std::vector<std::string> FontBook::bundled_font_paths() {
    std::vector<std::string> out;
    if (const char* env = std::getenv("URANIUM_FONT_DIR")) {
        out.push_back(std::string(env) + "/LiberationSans-Regular.ttf");
    }
#ifdef URANIUM_FONT_DIR
    out.push_back(std::string(URANIUM_FONT_DIR) + "/LiberationSans-Regular.ttf");
#endif
#ifdef URANIUM_INSTALL_FONT_DIR
    out.push_back(std::string(URANIUM_INSTALL_FONT_DIR) + "/LiberationSans-Regular.ttf");
#endif
    out.push_back("assets/fonts/LiberationSans-Regular.ttf");
    out.push_back("/usr/share/fonts/liberation-fonts/LiberationSans-Regular.ttf");
    out.push_back("/usr/share/fonts/TTF/LiberationSans-Regular.ttf");
    out.push_back("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    out.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    return out;
}

bool FontBook::register_font(const std::string& name, const std::string& path) {
    Font f;
    if (!f.load_file(path)) return false;
    for (auto& e : m_fonts) {
        if (e.name == name) {
            e.font = f;
            return true;
        }
    }
    m_fonts.push_back(Entry{name, f});
    if (m_default_name.empty()) m_default_name = name;
    return true;
}

bool FontBook::set_default(const std::string& name) {
    if (!has(name)) return false;
    m_default_name = name;
    return true;
}

bool FontBook::has(const std::string& name) const {
    for (const auto& e : m_fonts) if (e.name == name) return true;
    return false;
}

const Font& FontBook::get(const std::string& name) const {
    static const Font empty;
    for (const auto& e : m_fonts) if (e.name == name) return e.font;
    return empty;
}

const Font& FontBook::default_font() const {
    static const Font empty;
    for (const auto& e : m_fonts) if (e.name == m_default_name) return e.font;
    return empty;
}

std::vector<std::string> FontBook::names() const {
    std::vector<std::string> out;
    for (const auto& e : m_fonts) out.push_back(e.name);
    return out;
}
}
}
