#pragma once
#include "uranium/core/config.hpp"
#include "uranium/render/canvas.hpp"
namespace uranium {
namespace render {
class URANIUM_API TransformCanvas final : public Canvas {
public:
    TransformCanvas(Canvas& inner, float scale = 1.0f, Vec2 offset = Vec2{0.0f, 0.0f});
    int width() const override;
    int height() const override;
    void clear(Color c) override;
    void draw_line(Vec2 a, Vec2 b, Color c, float thickness = 1.0f) override;
    void draw_polyline(const std::vector<Vec2>& points, Color c, float thickness = 1.0f) override;
    void draw_polygon(const std::vector<Vec2>& points, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_rect(float x, float y, float w, float h, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_rounded_rect(float x, float y, float w, float h, float radius, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_circle(Vec2 center, float radius, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_ellipse(Vec2 center, float rx, float ry, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) override;
    void draw_arc(Vec2 center, float radius, float start_rad, float end_rad, Color stroke, float thickness = 1.0f) override;
    void draw_text(Vec2 position, const std::string& text, const struct Font& font, float px, Color c, TextAlign align = TextAlign::Left, Anchor anchor = Anchor::Baseline, float rotation_rad = 0.0f) override;
    Vec2 measure_text(const std::string& text, const struct Font& font, float px) const override;
    URANIUM_NODISCARD float scale() const { return m_scale; }
    URANIUM_NODISCARD Vec2 offset() const { return m_offset; }
private:
    Canvas* m_inner;
    float m_scale;
    Vec2 m_offset;
};
}
}
