#include "uranium/render/transform.hpp"
namespace uranium {
namespace render {
TransformCanvas::TransformCanvas(Canvas& inner, float scale, Vec2 offset): m_inner(&inner), m_scale(scale), m_offset(offset) {}
int TransformCanvas::width() const { return m_inner->width(); }
int TransformCanvas::height() const { return m_inner->height(); }
void TransformCanvas::clear(Color c) { m_inner->clear(c); }
void TransformCanvas::draw_line(Vec2 a, Vec2 b, Color c, float thickness) {
    m_inner->draw_line(a * m_scale + m_offset, b * m_scale + m_offset, c, thickness * m_scale);
}

void TransformCanvas::draw_polyline(const std::vector<Vec2>& points, Color c, float thickness) {
    std::vector<Vec2> out;
    out.reserve(points.size());
    for (const Vec2& p : points) out.push_back(p * m_scale + m_offset);
    m_inner->draw_polyline(out, c, thickness * m_scale);
}

void TransformCanvas::draw_polygon(const std::vector<Vec2>& points, Color fill, Color stroke, float thickness) {
    std::vector<Vec2> out;
    out.reserve(points.size());
    for (const Vec2& p : points) out.push_back(p * m_scale + m_offset);
    m_inner->draw_polygon(out, fill, stroke, thickness * m_scale);
}

void TransformCanvas::draw_rect(float x, float y, float w, float h, Color fill, Color stroke, float thickness) {
    m_inner->draw_rect(x * m_scale + m_offset.x, y * m_scale + m_offset.y, w * m_scale, h * m_scale, fill, stroke, thickness * m_scale);
}

void TransformCanvas::draw_rounded_rect(float x, float y, float w, float h, float radius, Color fill, Color stroke, float thickness) {
    m_inner->draw_rounded_rect(x * m_scale + m_offset.x, y * m_scale + m_offset.y, w * m_scale, h * m_scale, radius * m_scale, fill, stroke, thickness * m_scale);
}

void TransformCanvas::draw_circle(Vec2 center, float radius, Color fill, Color stroke, float thickness) {
    m_inner->draw_circle(center * m_scale + m_offset, radius * m_scale, fill, stroke, thickness * m_scale);
}

void TransformCanvas::draw_ellipse(Vec2 center, float rx, float ry, Color fill, Color stroke, float thickness) {
    m_inner->draw_ellipse(center * m_scale + m_offset, rx * m_scale, ry * m_scale, fill, stroke, thickness * m_scale);
}

void TransformCanvas::draw_arc(Vec2 center, float radius, float start_rad, float end_rad, Color stroke, float thickness) {
    m_inner->draw_arc(center * m_scale + m_offset, radius * m_scale, start_rad, end_rad, stroke, thickness * m_scale);
}

void TransformCanvas::draw_text(Vec2 position, const std::string& text, const Font& font, float px, Color c, TextAlign align, Anchor anchor, float rotation_rad) {
    m_inner->draw_text(position * m_scale + m_offset, text, font, px * m_scale, c, align, anchor, rotation_rad);
}

Vec2 TransformCanvas::measure_text(const std::string& text, const Font& font, float px) const {
    return m_inner->measure_text(text, font, px) * m_scale;
}
}
}
