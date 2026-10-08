#pragma once
#include "uranium/core/config.hpp"
#include "uranium/math/vector.hpp"
#include "uranium/render/color.hpp"
#include <string>
#include <vector>
namespace uranium {
namespace render {
using Vec2 = Vector<float, 2>;
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    constexpr Rect() = default;
    constexpr Rect(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}
    URANIUM_NODISCARD constexpr float right() const { return x + w; }
    URANIUM_NODISCARD constexpr float bottom() const { return y + h; }
    URANIUM_NODISCARD constexpr Vec2 center() const { return Vec2{x + w * 0.5f, y + h * 0.5f}; }
    URANIUM_NODISCARD constexpr bool contains(Vec2 p) const {
        return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
    }
    URANIUM_NODISCARD constexpr Rect expanded(float m) const { return Rect(x - m, y - m, w + 2 * m, h + 2 * m); }
};

enum class TextAlign : int { Left = 0, Center = 1, Right = 2 };
enum class Anchor : int { Baseline = 0, Middle = 1, Top = 2, Bottom = 3 };
class URANIUM_API Canvas {
public:
    virtual ~Canvas() = default;
    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual void clear(Color c) = 0;
    virtual void draw_line(Vec2 a, Vec2 b, Color c, float thickness = 1.0f) = 0;
    virtual void draw_polyline(const std::vector<Vec2>& points, Color c, float thickness = 1.0f) = 0;
    virtual void draw_polygon(const std::vector<Vec2>& points, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) = 0;
    virtual void draw_rect(float x, float y, float w, float h, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) = 0;
    virtual void draw_rounded_rect(float x, float y, float w, float h, float radius, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) = 0;
    virtual void draw_circle(Vec2 center, float radius, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) = 0;
    virtual void draw_ellipse(Vec2 center, float rx, float ry, Color fill, Color stroke = colors::transparent, float thickness = 1.0f) = 0;
    virtual void draw_arc(Vec2 center, float radius, float start_rad, float end_rad, Color stroke, float thickness = 1.0f) = 0;
    virtual void draw_text(Vec2 position, const std::string& text, const struct Font& font, float px, Color c, TextAlign align = TextAlign::Left, Anchor anchor = Anchor::Baseline, float rotation_rad = 0.0f) = 0;
    virtual Vec2 measure_text(const std::string& text, const struct Font& font, float px) const = 0;
    void draw_dashed_line(Vec2 a, Vec2 b, Color c, float thickness, float dash, float gap = -1.0f) {
        if (gap < 0.0f) gap = dash;
        float len = (b - a).norm();
        if (len < 1e-6f) return;
        Vec2 dir = (b - a) * (1.0f / len);
        float pos = 0.0f;
        bool on = true;
        while (pos < len) {
            float step = on ? dash : gap;
            float end = pos + step;
            if (end > len) end = len;
            if (on) {
                Vec2 p0 = a + dir * pos;
                Vec2 p1 = a + dir * end;
                draw_line(p0, p1, c, thickness);
            }
            pos = end;
            on = !on;
        }
    }

    void draw_dashed_polyline(const std::vector<Vec2>& points, Color c, float thickness, float dash, float gap = -1.0f) {
        for (std::size_t i = 0; i + 1 < points.size(); ++i) draw_dashed_line(points[i], points[i + 1], c, thickness, dash, gap);
    }
};
}
}
