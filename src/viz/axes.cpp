#include "uranium/viz/axes.hpp"
#include "uranium/core/error.hpp"
#include "uranium/render/font.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
namespace uranium {
namespace viz {
namespace {
using render::Anchor;
using render::Canvas;
using render::Font;
using render::FontBook;
using render::TextAlign;
using render::Vec2;
float nice_step(float lo, float hi, int target) {
    float span = hi - lo;
    if (!(span > 0.0f)) return 1.0f;
    if (target < 2) target = 2;
    float raw = span / static_cast<float>(target - 1);
    float mag = std::pow(10.0f, std::floor(std::log10(raw)));
    if (!(mag > 0.0f)) mag = 1.0f;
    float norm = raw / mag;
    float m = norm < 1.5f ? 1.0f : (norm < 3.0f ? 2.0f : (norm < 7.0f ? 5.0f : 10.0f));
    return m * mag;
}

std::vector<float> ticks_for(float lo, float hi, float step) {
    std::vector<float> out;
    if (!(step > 0.0f)) {
        out.push_back(lo);
        return out;
    }
    float start = std::ceil(lo / step - 1e-6f) * step;
    for (float v = start; v <= hi + step * 1e-3f; v += step) {
        if (std::fabs(v) < step * 1e-6f) v = 0.0f;
        out.push_back(v);
        if (out.size() > 64) break;
    }
    if (out.empty()) out.push_back(0.5f * (lo + hi));
    return out;
}

std::string trim_decimal_zeros(std::string s) {
    if (s.find('.') == std::string::npos) return s;
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
    return s;
}

std::string format_tick(float v, float step) {
    float mag = std::fabs(v);
    if (mag >= 1e6f || (mag > 0.0f && mag < 1e-4f)) {
        char buf[40];
        std::snprintf(buf, sizeof(buf), "%.1e", static_cast<double>(v));
        std::string s(buf);
        std::size_t e = s.find('e');
        if (e == std::string::npos) return s;
        std::string mant = trim_decimal_zeros(s.substr(0, e));
        std::string exp = s.substr(e + 1);
        if (exp.empty()) return mant;
        char sign = exp[0];
        std::string digits = exp.substr(1);
        while (digits.size() > 1 && digits[0] == '0') digits.erase(0, 1);
        if (sign == '+') return mant + "e" + digits;
        return mant + "e" + sign + digits;
    }
    int decimals = 0;
    if (step > 0.0f && step < 1.0f) {
        decimals = static_cast<int>(std::ceil(-std::log10(step) - 1e-9f));
        decimals = std::max(0, std::min(decimals, 12));
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v));
    return trim_decimal_zeros(buf);
}

bool clip_segment(Vec2& a, Vec2& b, const Rect& r) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float t0 = 0.0f;
    float t1 = 1.0f;
    auto step = [&](float p, float q) -> bool {
        if (std::fabs(p) < 1e-12f) return q >= 0.0f;
        float t = q / p;
        if (p < 0.0f) {
            if (t > t1) return false;
            if (t > t0) t0 = t;
        } else {
            if (t < t0) return false;
            if (t < t1) t1 = t;
        }
        return true;
    };
    if (!step(-dx, a.x - r.x)) return false;
    if (!step(dx, r.right() - a.x)) return false;
    if (!step(-dy, a.y - r.y)) return false;
    if (!step(dy, r.bottom() - a.y)) return false;
    Vec2 na{a.x + t0 * dx, a.y + t0 * dy};
    Vec2 nb{a.x + t1 * dx, a.y + t1 * dy};
    a = na;
    b = nb;
    return true;
}

std::vector<Vec2> clip_polygon(std::vector<Vec2> poly, const Rect& r) {
    if (poly.size() < 3) return {};
    for (int edge = 0; edge < 4; ++edge) {
        auto inside = [&](Vec2 p) {
            switch (edge) {
                case 0: return p.x >= r.x;
                case 1: return p.x <= r.right();
                case 2: return p.y >= r.y;
                default: return p.y <= r.bottom();
            }
        };
        auto intersect = [&](Vec2 a, Vec2 b) {
            if (edge < 2) {
                float xv = edge == 0 ? r.x : r.right();
                float t = (xv - a.x) / (b.x - a.x);
                return Vec2{xv, a.y + t * (b.y - a.y)};
            }
            float yv = edge == 2 ? r.y : r.bottom();
            float t = (yv - a.y) / (b.y - a.y);
            return Vec2{a.x + t * (b.x - a.x), yv};
        };
        std::vector<Vec2> out;
        if (poly.empty()) break;
        Vec2 prev = poly.back();
        for (Vec2 cur : poly) {
            bool ci = inside(cur);
            bool pi = inside(prev);
            if (ci) {
                if (!pi) out.push_back(intersect(prev, cur));
                out.push_back(cur);
            } else if (pi) {
                out.push_back(intersect(prev, cur));
            }
            prev = cur;
        }
        poly.swap(out);
    }
    if (poly.size() < 3) return {};
    return poly;
}

void draw_marker(Canvas& c, Vec2 pos, Marker m, float size, Color color) {
    switch (m) {
        case Marker::Circle:
            c.draw_circle(pos, size, color);
            break;
        case Marker::Square:
            c.draw_rect(pos.x - size, pos.y - size, size * 2.0f, size * 2.0f, color);
            break;
        case Marker::Cross:
            c.draw_line({pos.x - size, pos.y - size}, {pos.x + size, pos.y + size}, color, std::max(1.5f, size * 0.7f));
            c.draw_line({pos.x - size, pos.y + size}, {pos.x + size, pos.y - size}, color, std::max(1.5f, size * 0.7f));
            break;
        case Marker::Plus:
            c.draw_line({pos.x - size, pos.y}, {pos.x + size, pos.y}, color, std::max(1.5f, size * 0.7f));
            c.draw_line({pos.x, pos.y - size}, {pos.x, pos.y + size}, color, std::max(1.5f, size * 0.7f));
            break;
        case Marker::Diamond:
            c.draw_polygon({{pos.x, pos.y - size * 1.2f}, {pos.x + size * 1.2f, pos.y},
                            {pos.x, pos.y + size * 1.2f}, {pos.x - size * 1.2f, pos.y}}, color);
            break;
        case Marker::Triangle:
            c.draw_polygon({{pos.x, pos.y - size * 1.2f},
                            {pos.x + size * 1.1f, pos.y + size * 0.9f},
                            {pos.x - size * 1.1f, pos.y + size * 0.9f}}, color);
            break;
    }
}

float auto_bar_width(const std::vector<Vec2>& points) {
    if (points.size() < 2) return 0.6f;
    std::vector<float> xs;
    xs.reserve(points.size());
    for (const Vec2& p : points) xs.push_back(p.x);
    std::sort(xs.begin(), xs.end());
    float best = std::numeric_limits<float>::max();
    for (std::size_t i = 1; i < xs.size(); ++i) {
        float d = xs[i] - xs[i - 1];
        if (d > 1e-9f) best = std::min(best, d);
    }
    if (!(best < std::numeric_limits<float>::max())) return 0.6f;
    return best * 0.8f;
}

void draw_legend(Canvas& c, const Axes& ax, const Theme& th, float base, const Rect& R) {
    const Font& f = FontBook::instance().default_font();
    struct Entry {
        std::string label;
        Color color;
        SeriesKind kind;
        Marker marker;
        float line_width;
    };
    std::vector<Entry> entries;
    for (const Series& s : ax.series()) {
        if (!s.show_in_legend || s.label.empty()) continue;
        entries.push_back({s.label, s.color, s.kind, s.marker, s.line_width});
    }
    if (entries.empty()) return;
    float px = base * 0.85f;
    float pad = px * 0.7f;
    float row_h = px * 1.7f;
    float sw = px * 2.2f;
    float gap = px * 0.6f;
    float label_w = 0.0f;
    for (const Entry& e : entries) label_w = std::max(label_w, f.advance(e.label, px));
    float box_w = pad * 2.0f + sw + gap + label_w;
    float box_h = pad * 2.0f + static_cast<float>(entries.size()) * row_h;
    float inset = 8.0f;
    float x = R.right() - inset - box_w;
    float y = R.y + inset;
    switch (ax.legend_loc()) {
        case LegendLoc::UpperLeft:
            x = R.x + inset;
            break;
        case LegendLoc::LowerRight:
            y = R.bottom() - inset - box_h;
            break;
        case LegendLoc::LowerLeft:
            x = R.x + inset;
            y = R.bottom() - inset - box_h;
            break;
        case LegendLoc::UpperRight:
        default:
            break;
    }
    c.draw_rounded_rect(x, y, box_w, box_h, base * 0.35f, th.legend_background, th.legend_border, 1.0f);
    for (std::size_t i = 0; i < entries.size(); ++i) {
        const Entry& e = entries[i];
        float cy = y + pad + (static_cast<float>(i) + 0.5f) * row_h;
        float sx = x + pad;
        if (e.kind == SeriesKind::Scatter) {
            draw_marker(c, {sx + sw * 0.5f, cy}, e.marker, px * 0.45f, e.color);
        } else if (e.kind == SeriesKind::Bars) {
            c.draw_rect(sx + sw * 0.2f, cy - row_h * 0.3f, sw * 0.6f, row_h * 0.6f, e.color);
        } else {
            c.draw_line({sx, cy}, {sx + sw, cy}, e.color, std::max(2.0f, e.line_width));
        }
        c.draw_text({sx + sw + gap, cy}, e.label, f, px, th.text_color, TextAlign::Left, Anchor::Middle);
    }
}

}

Axes::Axes() = default;
Series& Axes::add(Series series) {
    if (series.color.a == 0) {
        series.color = render::colors::palette(static_cast<int>(m_series.size()));
    }
    m_series.push_back(std::move(series));
    return m_series.back();
}

Series& Axes::plot(const std::vector<float>& xs, const std::vector<float>& ys, std::string label) {
    if (xs.size() != ys.size()) throw ValueError("plot: x and y sizes differ");
    Series s;
    s.kind = SeriesKind::Line;
    s.points.reserve(xs.size());
    for (std::size_t i = 0; i < xs.size(); ++i) s.points.push_back({xs[i], ys[i]});
    s.label = std::move(label);
    return add(std::move(s));
}

Series& Axes::plot(const std::vector<Vec2>& points, std::string label) {
    Series s;
    s.kind = SeriesKind::Line;
    s.points = points;
    s.label = std::move(label);
    return add(std::move(s));
}

Series& Axes::scatter(const std::vector<float>& xs, const std::vector<float>& ys, std::string label) {
    if (xs.size() != ys.size()) throw ValueError("scatter: x and y sizes differ");
    Series s;
    s.kind = SeriesKind::Scatter;
    s.points.reserve(xs.size());
    for (std::size_t i = 0; i < xs.size(); ++i) s.points.push_back({xs[i], ys[i]});
    s.label = std::move(label);
    return add(std::move(s));
}

Series& Axes::scatter(const std::vector<Vec2>& points, std::string label) {
    Series s;
    s.kind = SeriesKind::Scatter;
    s.points = points;
    s.label = std::move(label);
    return add(std::move(s));
}

Series& Axes::bars(const std::vector<float>& xs, const std::vector<float>& heights, std::string label, float width) {
    if (xs.size() != heights.size()) throw ValueError("bars: x and height sizes differ");
    Series s;
    s.kind = SeriesKind::Bars;
    s.points.reserve(xs.size());
    for (std::size_t i = 0; i < xs.size(); ++i) s.points.push_back({xs[i], heights[i]});
    s.bar_width = width;
    s.label = std::move(label);
    return add(std::move(s));
}

Series& Axes::fill_between(const std::vector<float>& xs, const std::vector<float>& y_upper, const std::vector<float>& y_lower, std::string label) {
    if (xs.size() != y_upper.size() || xs.size() != y_lower.size()) throw ValueError("fill_between: sizes differ");
    Series s;
    s.kind = SeriesKind::Fill;
    s.points.reserve(xs.size());
    s.baseline_points.reserve(xs.size());
    for (std::size_t i = 0; i < xs.size(); ++i) {
        s.points.push_back({xs[i], y_upper[i]});
        s.baseline_points.push_back({xs[i], y_lower[i]});
    }
    s.label = std::move(label);
    return add(std::move(s));
}

Axes& Axes::axhline(float y, Color color, float thickness) {
    m_guides.push_back(GuideLine{false, y, color, thickness});
    return *this;
}

Axes& Axes::axvline(float x, Color color, float thickness) {
    m_guides.push_back(GuideLine{true, x, color, thickness});
    return *this;
}

Axes& Axes::title(std::string text) {
    m_title = std::move(text);
    return *this;
}

Axes& Axes::xlabel(std::string text) {
    m_xlabel = std::move(text);
    return *this;
}

Axes& Axes::ylabel(std::string text) {
    m_ylabel = std::move(text);
    return *this;
}

Axes& Axes::set_xlim(float lo, float hi) {
    if (!(hi > lo)) throw ValueError("set_xlim: hi must be > lo");
    m_xrange = {lo, hi};
    m_explicit_x = true;
    return *this;
}

Axes& Axes::set_ylim(float lo, float hi) {
    if (!(hi > lo)) throw ValueError("set_ylim: hi must be > lo");
    m_yrange = {lo, hi};
    m_explicit_y = true;
    return *this;
}

Axes& Axes::set_range(float x_lo, float x_hi, float y_lo, float y_hi) {
    set_xlim(x_lo, x_hi);
    set_ylim(y_lo, y_hi);
    return *this;
}

Axes& Axes::legend(bool show) {
    m_legend = show;
    return *this;
}

Axes& Axes::legend(LegendLoc loc) {
    m_legend = true;
    m_legend_loc = loc;
    return *this;
}

Axes& Axes::grid(bool show) {
    m_grid = show;
    return *this;
}

Axes& Axes::spines(Spines style) {
    m_spines = style;
    return *this;
}

Axes& Axes::annotate(std::string text, Vec2 data, Vec2 offset, TextAlign align) {
    Annotation a;
    a.text = std::move(text);
    a.data = data;
    a.offset = offset;
    a.align = align;
    m_annotations.push_back(std::move(a));
    return *this;
}

Vec2 Axes::to_pixel(Vec2 data) const {
    float sx = m_plot_rect.w / (m_xrange.span() > 1e-12f ? m_xrange.span() : 1.0f);
    float sy = m_plot_rect.h / (m_yrange.span() > 1e-12f ? m_yrange.span() : 1.0f);
    float fx = (data.x - m_xrange.lo) * sx;
    float fy = (data.y - m_yrange.lo) * sy;
    return {m_plot_rect.x + fx, m_plot_rect.bottom() - fy};
}

Vec2 Axes::to_data(Vec2 pixel) const {
    float sx = m_plot_rect.w / (m_xrange.span() > 1e-12f ? m_xrange.span() : 1.0f);
    float sy = m_plot_rect.h / (m_yrange.span() > 1e-12f ? m_yrange.span() : 1.0f);
    float dx = m_xrange.lo + (pixel.x - m_plot_rect.x) / (sx > 1e-12f ? sx : 1.0f);
    float dy = m_yrange.lo + (m_plot_rect.bottom() - pixel.y) / (sy > 1e-12f ? sy : 1.0f);
    return {dx, dy};
}

void Axes::autoscale() {
    float xlo = std::numeric_limits<float>::max();
    float xhi = std::numeric_limits<float>::lowest();
    float ylo = std::numeric_limits<float>::max();
    float yhi = std::numeric_limits<float>::lowest();
    bool any = false;
    auto acc = [&](float x, float y) {
        any = true;
        xlo = std::min(xlo, x);
        xhi = std::max(xhi, x);
        ylo = std::min(ylo, y);
        yhi = std::max(yhi, y);
    };
    for (const Series& s : m_series) {
        if (s.kind == SeriesKind::Bars) {
            float w = s.bar_width > 0.0f ? s.bar_width : auto_bar_width(s.points);
            for (const Vec2& p : s.points) {
                acc(p.x - w * 0.5f, std::min(0.0f, p.y));
                acc(p.x + w * 0.5f, std::max(0.0f, p.y));
            }
        } else {
            for (const Vec2& p : s.points) acc(p.x, p.y);
            for (const Vec2& p : s.baseline_points) acc(p.x, p.y);
        }
    }
    if (!any) {
        xlo = 0.0f;
        xhi = 1.0f;
        ylo = 0.0f;
        yhi = 1.0f;
    }
    auto pad = [](float& lo, float& hi) {
        if (!(hi > lo)) {
            lo -= 1.0f;
            hi += 1.0f;
        } else {
            float d = hi - lo;
            lo -= d * 0.05f;
            hi += d * 0.05f;
        }
    };
    if (!m_explicit_x) {
        pad(xlo, xhi);
        m_xrange = {xlo, xhi};
    }
    if (!m_explicit_y) {
        pad(ylo, yhi);
        m_yrange = {ylo, yhi};
    }
}

Axes::TickInfo Axes::x_ticks(const Rect& cell) const {
    TickInfo info;
    int target = static_cast<int>(std::max(2.0f, std::min(12.0f, cell.w / 90.0f)));
    info.step = nice_step(m_xrange.lo, m_xrange.hi, target);
    info.values = ticks_for(m_xrange.lo, m_xrange.hi, info.step);
    for (float v : info.values) info.labels.push_back(format_tick(v, info.step));
    return info;
}

Axes::TickInfo Axes::y_ticks(const Rect& cell) const {
    TickInfo info;
    int target = static_cast<int>(std::max(2.0f, std::min(10.0f, cell.h / 55.0f)));
    info.step = nice_step(m_yrange.lo, m_yrange.hi, target);
    info.values = ticks_for(m_yrange.lo, m_yrange.hi, info.step);
    for (float v : info.values) info.labels.push_back(format_tick(v, info.step));
    return info;
}

Margins Axes::measure(float base_px, const Rect& cell) const {
    Margins m;
    const Font& f = FontBook::instance().default_font();
    float tick_px = base_px * 0.85f;
    TickInfo yt = y_ticks(cell);
    float ylab_w = 0.0f;
    for (const std::string& l : yt.labels) ylab_w = std::max(ylab_w, f.advance(l, tick_px));
    m.left = ylab_w + 10.0f;
    if (!m_ylabel.empty()) m.left += tick_px * 1.6f;
    m.left = std::max(m.left, 32.0f);
    m.right = std::max(12.0f, base_px * 0.7f);
    m.top = std::max(16.0f, base_px);
    if (!m_title.empty()) m.top += tick_px + 8.0f;
    m.bottom = 6.0f + tick_px;
    if (!m_xlabel.empty()) m.bottom += tick_px + 8.0f;
    m.bottom = std::max(m.bottom, 26.0f);
    m.left = std::min(m.left, cell.w * 0.4f);
    m.right = std::min(m.right, cell.w * 0.2f);
    m.top = std::min(m.top, cell.h * 0.25f);
    m.bottom = std::min(m.bottom, cell.h * 0.4f);
    return m;
}

void Axes::render(Canvas& c, const Theme& th, float base_px, const Rect& cell) const {
    const Rect& R = m_plot_rect;
    if (R.w < 4.0f || R.h < 4.0f) return;
    const Font& f = FontBook::instance().default_font();
    float tick_px = base_px * 0.85f;
    c.draw_rect(R.x, R.y, R.w, R.h, th.plot_background);
    TickInfo xt = x_ticks(cell);
    TickInfo yt = y_ticks(cell);
    if (m_grid) {
        for (float v : yt.values) {
            float y = to_pixel({m_xrange.lo, v}).y;
            if (y >= R.y - 0.5f && y <= R.bottom() + 0.5f) c.draw_dashed_line({R.x, y}, {R.right(), y}, th.grid_color, 1.0f, 4.0f, 4.0f);
        }
        for (float v : xt.values) {
            float x = to_pixel({v, m_yrange.lo}).x;
            if (x >= R.x - 0.5f && x <= R.right() + 0.5f) c.draw_dashed_line({x, R.y}, {x, R.bottom()}, th.grid_color, 1.0f, 4.0f, 4.0f);
        }
    }

    float ylab_w = 0.0f;
    for (const std::string& l : yt.labels) ylab_w = std::max(ylab_w, f.advance(l, tick_px));
    for (std::size_t i = 0; i < xt.values.size(); ++i) {
        float x = to_pixel({xt.values[i], m_yrange.lo}).x;
        c.draw_line({x, R.bottom()}, {x, R.bottom() + 4.0f}, th.axis_color, 1.2f);
        c.draw_text({x, R.bottom() + 7.0f + tick_px * 0.8f}, xt.labels[i], f, tick_px, th.text_color, TextAlign::Center, Anchor::Bottom);
    }
    for (std::size_t i = 0; i < yt.values.size(); ++i) {
        float y = to_pixel({m_xrange.lo, yt.values[i]}).y;
        c.draw_line({R.x - 4.0f, y}, {R.x, y}, th.axis_color, 1.2f);
        c.draw_text({R.x - 8.0f, y}, yt.labels[i], f, tick_px, th.text_color, TextAlign::Right, Anchor::Middle);
    }

    if (m_spines == Spines::Box) {
        c.draw_line({R.x, R.y}, {R.right(), R.y}, th.axis_color, 1.4f);
        c.draw_line({R.right(), R.y}, {R.right(), R.bottom()}, th.axis_color, 1.0f);
        c.draw_line({R.right(), R.bottom()}, {R.x, R.bottom()}, th.axis_color, 1.4f);
        c.draw_line({R.x, R.bottom()}, {R.x, R.y}, th.axis_color, 1.4f);
    } else {
        c.draw_line({R.x, R.bottom()}, {R.right(), R.bottom()}, th.axis_color, 1.4f);
        c.draw_line({R.x, R.y}, {R.x, R.bottom()}, th.axis_color, 1.4f);
    }

    if (!m_title.empty())
        c.draw_text({R.center().x, R.y - 9.0f}, m_title, f, base_px, th.text_color, TextAlign::Center, Anchor::Bottom);
    if (!m_xlabel.empty())
        c.draw_text({R.center().x, R.bottom() + 7.0f + tick_px + 6.0f}, m_xlabel, f, base_px, th.text_color, TextAlign::Center, Anchor::Top);
    if (!m_ylabel.empty()) {
        float yl_x = R.x - 8.0f - ylab_w - base_px * 0.8f;
        c.draw_text({yl_x, R.center().y}, m_ylabel, f, base_px, th.text_color, TextAlign::Center, Anchor::Middle, -1.57079632679f);
    }

    for (const GuideLine& g : m_guides) {
        Color col = g.color.a != 0 ? g.color : th.axis_color;
        if (!g.vertical) {
            float y = to_pixel({m_xrange.lo, g.value}).y;
            Vec2 a{R.x, y};
            Vec2 b{R.right(), y};
            if (clip_segment(a, b, R)) c.draw_line(a, b, col, g.thickness);
        } else {
            float x = to_pixel({g.value, m_yrange.lo}).x;
            Vec2 a{x, R.y};
            Vec2 b{x, R.bottom()};
            if (clip_segment(a, b, R)) c.draw_line(a, b, col, g.thickness);
        }
    }

    for (const Series& s : m_series) {
        if (s.points.empty()) continue;
        if (s.kind == SeriesKind::Line) {
            if (s.points.size() == 1) {
                Vec2 px = to_pixel(s.points[0]);
                if (R.contains(px))
                    c.draw_circle(px, std::max(2.0f, s.line_width * 1.4f), s.color);
            }
            for (std::size_t i = 0; i + 1 < s.points.size(); ++i) {
                Vec2 a = to_pixel(s.points[i]);
                Vec2 b = to_pixel(s.points[i + 1]);
                if (clip_segment(a, b, R)) c.draw_line(a, b, s.color, s.line_width);
            }
        } else if (s.kind == SeriesKind::Scatter) {
            for (const Vec2& p : s.points) {
                Vec2 px = to_pixel(p);
                if (R.contains(px)) draw_marker(c, px, s.marker, s.marker_size, s.color);
            }
        } else if (s.kind == SeriesKind::Bars) {
            float w = s.bar_width > 0.0f ? s.bar_width : auto_bar_width(s.points);
            float base_v = m_yrange.clamped(0.0f);
            for (const Vec2& p : s.points) {
                float x0 = to_pixel({p.x - w * 0.5f, p.x}).x;
                float x1 = to_pixel({p.x + w * 0.5f, p.x}).x;
                float y0 = to_pixel({0.0f, p.y}).y;
                float y1 = to_pixel({0.0f, base_v}).y;
                float rx = std::max(R.x, std::min(x0, x1));
                float rr = std::min(R.right(), std::max(x0, x1));
                float ry = std::max(R.y, std::min(y0, y1));
                float rb = std::min(R.bottom(), std::max(y0, y1));
                if (rr > rx && rb > ry) c.draw_rect(rx, ry, rr - rx, rb - ry, s.color);
            }
        } else if (s.kind == SeriesKind::Fill) {
            std::vector<Vec2> upper;
            std::vector<Vec2> lower;
            upper.reserve(s.points.size());
            lower.reserve(s.baseline_points.size());
            for (const Vec2& p : s.points) upper.push_back(to_pixel(p));
            for (const Vec2& p : s.baseline_points) lower.push_back(to_pixel(p));
            std::vector<Vec2> poly = upper;
            poly.insert(poly.end(), lower.rbegin(), lower.rend());
            std::vector<Vec2> clipped = clip_polygon(std::move(poly), R);
            if (clipped.size() >= 3) c.draw_polygon(clipped, s.color.with_alpha(110));
            for (std::size_t i = 0; i + 1 < upper.size(); ++i) {
                Vec2 a = upper[i];
                Vec2 b = upper[i + 1];
                if (clip_segment(a, b, R)) c.draw_line(a, b, s.color.with_alpha(200), s.line_width);
            }
            for (std::size_t i = 0; i + 1 < lower.size(); ++i) {
                Vec2 a = lower[i];
                Vec2 b = lower[i + 1];
                if (clip_segment(a, b, R)) c.draw_line(a, b, s.color.with_alpha(200), s.line_width);
            }
        }
    }

    for (const Annotation& a : m_annotations) {
        Color col = a.color.a != 0 ? a.color : th.text_color;
        Vec2 p = to_pixel(a.data);
        Vec2 tp = p + a.offset;
        if (a.show_point) c.draw_circle(p, 3.0f, Color{0, 0, 0, 0}, col, 1.6f);
        if (a.offset.norm() > 10.0f) c.draw_line(p, tp, col.with_alpha(140), 1.0f);
        Anchor anchor = Anchor::Middle;
        if (a.offset.y < -2.0f) anchor = Anchor::Bottom;
        else if (a.offset.y > 2.0f) anchor = Anchor::Top;
        c.draw_text(tp, a.text, f, base_px * 0.9f, col, a.align, anchor);
    }

    if (m_legend) draw_legend(c, *this, th, base_px, R);
}
}
}
