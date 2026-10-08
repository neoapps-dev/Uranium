#include "uranium/render/raster.hpp"
#include "uranium/render/font.hpp"
#include "../../third_party/stb/stb_image_write.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
namespace uranium {
namespace render {
namespace {
inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
float distance_to_segment(Vec2 p, Vec2 a, Vec2 b) {
    Vec2 ab = b - a;
    float len2 = ab.x * ab.x + ab.y * ab.y;
    float t = 0.0f;
    if (len2 > 1e-12f) t = clampf(((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / len2, 0.0f, 1.0f);
    float dx = p.x - (a.x + ab.x * t);
    float dy = p.y - (a.y + ab.y * t);
    return std::sqrt(dx * dx + dy * dy);
}

bool point_in_polygon(Vec2 p, const std::vector<Vec2>& poly) {
    bool inside = false;
    std::size_t n = poly.size();
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        const Vec2& a = poly[i];
        const Vec2& b = poly[j];
        if ((a.y > p.y) != (b.y > p.y)) {
            float x = (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x;
            if (p.x < x) inside = !inside;
        }
    }
    return inside;
}

int next_cp(const std::string& s, std::size_t& i, std::size_t& len) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    int cp = c;
    len = 1;
    if (c < 0x80) {
        cp = c;
    } else if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
        cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
        len = 2;
    } else if ((c & 0xF0) == 0xE0 && i + 2 < s.size()) {
        cp = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i + 2]) & 0x3F);
        len = 3;
    } else if ((c & 0xF8) == 0xF0 && i + 3 < s.size()) {
        cp = ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12) | ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i + 3]) & 0x3F);
        len = 4;
    } else {
        cp = 0x3F;
    }
    return cp;
}
}

RasterCanvas::RasterCanvas(int width, int height, Color background): m_width(std::max(1, width)), m_height(std::max(1, height)), m_buffer(static_cast<std::size_t>(m_width) * m_height * 4, 0) {
    clear(background);
}

void RasterCanvas::clear(Color c) {
    for (std::size_t i = 0; i + 3 < m_buffer.size(); i += 4) {
        m_buffer[i] = c.r;
        m_buffer[i + 1] = c.g;
        m_buffer[i + 2] = c.b;
        m_buffer[i + 3] = c.a;
    }
}

void RasterCanvas::blend_pixel(int x, int y, Color c, float coverage) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) return;
    float a = (static_cast<float>(c.a) / 255.0f) * coverage;
    if (a <= 0.0f) return;
    if (a > 1.0f) a = 1.0f;
    float ia = 1.0f - a;
    std::uint8_t* px = &m_buffer[(static_cast<std::size_t>(y) * m_width + x) * 4];
    px[0] = static_cast<std::uint8_t>(c.r * a + px[0] * ia + 0.5f);
    px[1] = static_cast<std::uint8_t>(c.g * a + px[1] * ia + 0.5f);
    px[2] = static_cast<std::uint8_t>(c.b * a + px[2] * ia + 0.5f);
    px[3] = static_cast<std::uint8_t>(255.0f * a + px[3] * ia + 0.5f);
}

void RasterCanvas::set_pixel(int x, int y, Color c) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) return;
    std::uint8_t* px = &m_buffer[(static_cast<std::size_t>(y) * m_width + x) * 4];
    px[0] = c.r;
    px[1] = c.g;
    px[2] = c.b;
    px[3] = c.a;
}

Color RasterCanvas::at(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) return colors::transparent;
    const std::uint8_t* px = &m_buffer[(static_cast<std::size_t>(y) * m_width + x) * 4];
    return Color(px[0], px[1], px[2], px[3]);
}

void RasterCanvas::draw_line(Vec2 a, Vec2 b, Color c, float thickness) {
    if (c.a == 0) return;
    float half = std::max(0.0f, thickness) * 0.5f;
    int minx = static_cast<int>(std::floor(std::min(a.x, b.x) - half - 1.0f));
    int maxx = static_cast<int>(std::ceil(std::max(a.x, b.x) + half + 1.0f));
    int miny = static_cast<int>(std::floor(std::min(a.y, b.y) - half - 1.0f));
    int maxy = static_cast<int>(std::ceil(std::max(a.y, b.y) + half + 1.0f));
    minx = std::max(0, minx);
    miny = std::max(0, miny);
    maxx = std::min(m_width - 1, maxx);
    maxy = std::min(m_height - 1, maxy);
    for (int y = miny; y <= maxy; ++y) {
        for (int x = minx; x <= maxx; ++x) {
            Vec2 p{x + 0.5f, y + 0.5f};
            float d = distance_to_segment(p, a, b);
            float cov = clampf(half + 0.5f - d, 0.0f, 1.0f);
            if (cov > 0.0f) blend_pixel(x, y, c, cov);
        }
    }
}

void RasterCanvas::draw_polyline(const std::vector<Vec2>& points, Color c, float thickness) {
    for (std::size_t i = 0; i + 1 < points.size(); ++i) draw_line(points[i], points[i + 1], c, thickness);
}

void RasterCanvas::draw_polygon(const std::vector<Vec2>& points, Color fill, Color stroke, float thickness) {
    if (points.size() < 3) return;
    float minx = points[0].x, maxx = points[0].x, miny = points[0].y, maxy = points[0].y;
    for (const auto& p : points) {
        minx = std::min(minx, p.x);
        maxx = std::max(maxx, p.x);
        miny = std::min(miny, p.y);
        maxy = std::max(maxy, p.y);
    }
    if (fill.a > 0) {
        int x0 = std::max(0, static_cast<int>(std::floor(minx)));
        int x1 = std::min(m_width - 1, static_cast<int>(std::ceil(maxx)));
        int y0 = std::max(0, static_cast<int>(std::floor(miny)));
        int y1 = std::min(m_height - 1, static_cast<int>(std::ceil(maxy)));
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                int hits = 0;
                for (int sy = 0; sy < 4; ++sy) {
                    float py = y + (sy + 0.5f) * 0.25f;
                    for (int sx = 0; sx < 4; ++sx) {
                        float px = x + (sx + 0.5f) * 0.25f;
                        if (point_in_polygon(Vec2{px, py}, points)) ++hits;
                    }
                }
                if (hits > 0) blend_pixel(x, y, fill, static_cast<float>(hits) / 16.0f);
            }
        }
    }
    if (stroke.a > 0) {
        std::vector<Vec2> closed = points;
        closed.push_back(points.front());
        draw_polyline(closed, stroke, thickness);
    }
}

void RasterCanvas::draw_rect(float x, float y, float w, float h, Color fill, Color stroke, float thickness) {
    if (fill.a > 0) {
        int x0 = std::max(0, static_cast<int>(std::floor(x)));
        int x1 = std::min(m_width - 1, static_cast<int>(std::ceil(x + w)) - 1);
        int y0 = std::max(0, static_cast<int>(std::floor(y)));
        int y1 = std::min(m_height - 1, static_cast<int>(std::ceil(y + h)) - 1);
        for (int yy = y0; yy <= y1; ++yy) {
            float py = yy + 0.5f;
            float cy = clampf(std::min(py + 0.5f, y + h) - std::max(py - 0.5f, y), 0.0f, 1.0f);
            if (cy <= 0.0f) continue;
            for (int xx = x0; xx <= x1; ++xx) {
                float px = xx + 0.5f;
                float cx = clampf(std::min(px + 0.5f, x + w) - std::max(px - 0.5f, x), 0.0f, 1.0f);
                float cov = cx * cy;
                if (cov > 0.0f) blend_pixel(xx, yy, fill, cov);
            }
        }
    }
    if (stroke.a > 0 && thickness > 0.0f) {
        draw_line({x, y}, {x + w, y}, stroke, thickness);
        draw_line({x + w, y}, {x + w, y + h}, stroke, thickness);
        draw_line({x + w, y + h}, {x, y + h}, stroke, thickness);
        draw_line({x, y + h}, {x, y}, stroke, thickness);
    }
}

void RasterCanvas::draw_rounded_rect(float x, float y, float w, float h, float radius, Color fill, Color stroke, float thickness) {
    float r = std::max(0.0f, std::min(radius, std::min(w, h) * 0.5f));
    if (r <= 0.0f) {
        draw_rect(x, y, w, h, fill, stroke, thickness);
        return;
    }
    float half_t = thickness * 0.5f;
    int x0 = std::max(0, static_cast<int>(std::floor(x - thickness)));
    int x1 = std::min(m_width - 1, static_cast<int>(std::ceil(x + w + thickness)));
    int y0 = std::max(0, static_cast<int>(std::floor(y - thickness)));
    int y1 = std::min(m_height - 1, static_cast<int>(std::ceil(y + h + thickness)));
    Vec2 center{x + w * 0.5f, y + h * 0.5f};
    Vec2 half{w * 0.5f - r, h * 0.5f - r};
    for (int yy = y0; yy <= y1; ++yy) {
        for (int xx = x0; xx <= x1; ++xx) {
            Vec2 p{xx + 0.5f, yy + 0.5f};
            Vec2 q{std::fabs(p.x - center.x) - half.x, std::fabs(p.y - center.y) - half.y};
            float qx = std::max(q.x, 0.0f);
            float qy = std::max(q.y, 0.0f);
            float d = std::sqrt(qx * qx + qy * qy) + std::min(std::max(q.x, q.y), 0.0f) - r;
            if (fill.a > 0) {
                float cov = clampf(0.5f - d, 0.0f, 1.0f);
                if (cov > 0.0f) blend_pixel(xx, yy, fill, cov);
            }
            if (stroke.a > 0 && thickness > 0.0f) {
                float cov = clampf(half_t + 0.5f - std::fabs(d), 0.0f, 1.0f);
                if (cov > 0.0f) blend_pixel(xx, yy, stroke, cov);
            }
        }
    }
}

void RasterCanvas::draw_circle(Vec2 center, float radius, Color fill, Color stroke, float thickness) {
    if (radius <= 0.0f) return;
    float half_t = thickness * 0.5f;
    int x0 = std::max(0, static_cast<int>(std::floor(center.x - radius - thickness)));
    int x1 = std::min(m_width - 1, static_cast<int>(std::ceil(center.x + radius + thickness)));
    int y0 = std::max(0, static_cast<int>(std::floor(center.y - radius - thickness)));
    int y1 = std::min(m_height - 1, static_cast<int>(std::ceil(center.y + radius + thickness)));
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            float dx = x + 0.5f - center.x;
            float dy = y + 0.5f - center.y;
            float d = std::sqrt(dx * dx + dy * dy) - radius;
            if (fill.a > 0) {
                float cov = clampf(0.5f - d, 0.0f, 1.0f);
                if (cov > 0.0f) blend_pixel(x, y, fill, cov);
            }
            if (stroke.a > 0 && thickness > 0.0f) {
                float cov = clampf(half_t + 0.5f - std::fabs(d), 0.0f, 1.0f);
                if (cov > 0.0f) blend_pixel(x, y, stroke, cov);
            }
        }
    }
}

void RasterCanvas::draw_ellipse(Vec2 center, float rx, float ry, Color fill, Color stroke, float thickness) {
    if (rx <= 0.0f || ry <= 0.0f) return;
    float half_t = thickness * 0.5f;
    float big = std::max(rx, ry);
    int x0 = std::max(0, static_cast<int>(std::floor(center.x - rx - thickness)));
    int x1 = std::min(m_width - 1, static_cast<int>(std::ceil(center.x + rx + thickness)));
    int y0 = std::max(0, static_cast<int>(std::floor(center.y - ry - thickness)));
    int y1 = std::min(m_height - 1, static_cast<int>(std::ceil(center.y + ry + thickness)));
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            float dx = (x + 0.5f - center.x) / rx;
            float dy = (y + 0.5f - center.y) / ry;
            float d = (std::sqrt(dx * dx + dy * dy) - 1.0f) * std::min(rx, ry);
            if (std::fabs(d) > big) continue;
            if (fill.a > 0) {
                float cov = clampf(0.5f - d, 0.0f, 1.0f);
                if (cov > 0.0f) blend_pixel(x, y, fill, cov);
            }
            if (stroke.a > 0 && thickness > 0.0f) {
                float cov = clampf(half_t + 0.5f - std::fabs(d), 0.0f, 1.0f);
                if (cov > 0.0f) blend_pixel(x, y, stroke, cov);
            }
        }
    }
}

void RasterCanvas::draw_arc(Vec2 center, float radius, float start_rad, float end_rad, Color stroke, float thickness) {
    if (radius <= 0.0f || stroke.a == 0) return;
    float span = end_rad - start_rad;
    if (std::fabs(span) < 1e-6f) return;
    int n = std::max(8, static_cast<int>(std::fabs(span) * radius * 0.5f) + 2);
    n = std::min(n, 512);
    std::vector<Vec2> pts;
    pts.reserve(static_cast<std::size_t>(n) + 1);
    for (int i = 0; i <= n; ++i) {
        float t = start_rad + span * (static_cast<float>(i) / n);
        pts.push_back(center + Vec2{std::cos(t), std::sin(t)} * radius);
    }
    draw_polyline(pts, stroke, thickness);
}

void RasterCanvas::draw_text(Vec2 position, const std::string& text, const Font& font, float px, Color c, TextAlign align, Anchor anchor, float rotation_rad) {
    if (!font.loaded() || text.empty() || c.a == 0) return;
    float asc = font.ascent(px);
    float desc = font.descent(px);
    float line_h = font.line_height(px);
    float rel_y = 0.0f;
    switch (anchor) {
        case Anchor::Middle: rel_y = (asc - desc) * 0.5f; break;
        case Anchor::Top: rel_y = asc; break;
        case Anchor::Bottom: rel_y = -desc; break;
        case Anchor::Baseline: rel_y = 0.0f; break;
    }
    float cs = std::cos(rotation_rad);
    float sn = std::sin(rotation_rad);
    std::size_t line_start = 0;
    int line_index = 0;
    auto render_line = [&](const std::string& line) {
        if (line.empty()) {
            ++line_index;
            return;
        }
        float w = font.advance(line, px);
        float rel_x = 0.0f;
        if (align == TextAlign::Center) rel_x = -w * 0.5f;
        else if (align == TextAlign::Right) rel_x = -w;
        float baseline = rel_y + line_index * line_h;
        float cx = 0.0f;
        int prev = 0;
        std::size_t i = 0;
        while (i < line.size()) {
            std::size_t len = 1;
            int cp = next_cp(line, i, len);
            if (prev != 0) cx += font.kerning(prev, cp, px);
            GlyphBitmap g;
            (void)font.rasterize(cp, px, g);
            for (int gy = 0; gy < g.height; ++gy) {
                for (int gx = 0; gx < g.width; ++gx) {
                    std::uint8_t cov = g.coverage[static_cast<std::size_t>(gy) * g.width + gx];
                    if (cov == 0) continue;
                    float lx = rel_x + cx + g.x_offset + gx + 0.5f;
                    float ly = baseline + g.y_offset + gy + 0.5f;
                    float wx = position.x + lx * cs - ly * sn;
                    float wy = position.y + lx * sn + ly * cs;
                    blend_pixel(static_cast<int>(std::floor(wx)), static_cast<int>(std::floor(wy)), c, static_cast<float>(cov) / 255.0f);
                }
            }
            cx += g.advance;
            prev = cp;
            i += len;
        }
        ++line_index;
    };

    while (line_start <= text.size()) {
        std::size_t nl = text.find('\n', line_start);
        std::string line = nl == std::string::npos ? text.substr(line_start) : text.substr(line_start, nl - line_start);
        render_line(line);
        if (nl == std::string::npos) break;
        line_start = nl + 1;
    }
}

Vec2 RasterCanvas::measure_text(const std::string& text, const Font& font, float px) const {
    if (!font.loaded()) return Vec2{0.0f, px};
    float w = 0.0f;
    std::size_t line_start = 0;
    int lines = 1;
    while (line_start <= text.size()) {
        std::size_t nl = text.find('\n', line_start);
        std::string line = nl == std::string::npos ? text.substr(line_start) : text.substr(line_start, nl - line_start);
        w = std::max(w, font.advance(line, px));
        ++lines;
        if (nl == std::string::npos) break;
        line_start = nl + 1;
    }
    return Vec2{w, font.line_height(px) * static_cast<float>(std::max(1, lines - 1))};
}

bool RasterCanvas::save_png(const std::string& path) const {
    return stbi_write_png(path.c_str(), m_width, m_height, 4, m_buffer.data(), m_width * 4) != 0;
}

bool RasterCanvas::save_bmp(const std::string& path) const {
    return stbi_write_bmp(path.c_str(), m_width, m_height, 4, m_buffer.data()) != 0;
}
}
}
