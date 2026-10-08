#pragma once
#include "uranium/core/config.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
namespace uranium {
namespace render {
struct GlyphBitmap {
    int width = 0;
    int height = 0;
    int x_offset = 0;
    int y_offset = 0;
    float advance = 0.0f;
    std::vector<std::uint8_t> coverage;
};

class URANIUM_API Font {
public:
    Font();
    Font(const Font& other);
    Font(Font&& other) noexcept;
    Font& operator=(const Font& other);
    Font& operator=(Font&& other) noexcept;
    ~Font();
    bool load_file(const std::string& path);
    bool load_memory(const std::uint8_t* data, std::size_t size);
    URANIUM_NODISCARD bool loaded() const;
    URANIUM_NODISCARD float ascent(float px) const;
    URANIUM_NODISCARD float descent(float px) const;
    URANIUM_NODISCARD float line_height(float px) const;
    URANIUM_NODISCARD float advance(const std::string& text, float px) const;
    URANIUM_NODISCARD float measure_width(const std::string& text, float px) const;
    URANIUM_NODISCARD float kerning(int prev_cp, int next_cp, float px) const;
    URANIUM_NODISCARD bool rasterize(int codepoint, float px, GlyphBitmap& out) const;
    URANIUM_NODISCARD const std::string& name() const;

private:
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

class URANIUM_API FontBook {
public:
    static FontBook& instance();
    bool register_font(const std::string& name, const std::string& path);
    bool set_default(const std::string& name);
    URANIUM_NODISCARD const Font& get(const std::string& name) const;
    URANIUM_NODISCARD const Font& default_font() const;
    URANIUM_NODISCARD bool has(const std::string& name) const;
    URANIUM_NODISCARD std::vector<std::string> names() const;
    URANIUM_NODISCARD static std::vector<std::string> bundled_font_paths();

private:
    FontBook();
    struct Entry {
        std::string name;
        Font font;
    };
    std::vector<Entry> m_fonts;
    std::string m_default_name;
};
}
}
