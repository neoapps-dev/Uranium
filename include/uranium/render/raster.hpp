#pragma once
#include "uranium/core/config.hpp"
#include "uranium/render/canvas.hpp"
#include <cstdint>
#include <string>
#include <vector>
namespace uranium {
namespace render {
class URANIUM_API RasterCanvas final : public Canvas {
public:
    RasterCanvas(int width, int height, Color background = colors::white);
    int width() const override { return m_width; }
    int height() const override { return m_height; }
    void clear(Color c) override;
    void draw_line(Vec2 a, Vec2 b, Color c, float thickness = 1.0f) override;
    void draw_polyline(const std::vector<Vec2>& points, Color c, float thickness = 1.0f) override;
    void draw_polygon(const std::vector<Vec2>& points, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_rect(float x, float y, float w, float h, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_rounded_rect(float x, float y, float w, float h, float radius, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_circle(Vec2 center, float radius, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_ellipse(Vec2 center, float rx, float ry, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_arc(Vec2 center, float radius, float start_rad, float end_rad, Color stroke, float thickness = 1.0f) override;
    void draw_text(Vec2 position, const std::string& text, const Font& font, float px, Color c, TextAlign align = TextAlign::Left, Anchor anchor = Anchor::Baseline, float rotation_rad = 0.0f) override;
    Vec2 measure_text(const std::string& text, const Font& font, float px) const override;
    void blend_pixel(int x, int y, Color c, float coverage = 1.0f);
    void set_pixel(int x, int y, Color c);
    URANIUM_NODISCARD Color at(int x, int y) const;
    URANIUM_NODISCARD const std::vector<std::uint8_t>& pixels() const { return m_buffer; }
    bool save_png(const std::string& path) const;
    bool save_bmp(const std::string& path) const;

private:
    int m_width;
    int m_height;
    std::vector<std::uint8_t> m_buffer;
};
}
}
