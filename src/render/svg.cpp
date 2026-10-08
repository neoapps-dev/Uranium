#include "uranium/render/svg.hpp"
#include "uranium/render/font.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
namespace uranium {
namespace render {
namespace {
std::string n(float v) {
    char buf[40];
    std::snprintf(buf, sizeof(buf), "%.2f", v);
    std::string s = buf;
    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
    }
    if (s == "-0") s = "0";
    return s;
}

std::string esc(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default: out.push_back(c);
        }
    }
    return out;
}

std::string points_attr(const std::vector<Vec2>& pts) {
    std::string out;
    out.reserve(pts.size() * 16);
    for (std::size_t i = 0; i < pts.size(); ++i) {
        if (i) out += " ";
        out += n(pts[i].x) + "," + n(pts[i].y);
    }
    return out;
}

std::string stroke_attr(Color c, float thickness) {
    if (c.a == 0 || thickness <= 0.0f) return " stroke=\"none\"";
    return " stroke=\"" + c.css() + "\" stroke-width=\"" + n(thickness) +
           "\" stroke-linecap=\"round\" stroke-linejoin=\"round\"";
}

std::string fill_attr(Color c) {
    if (c.a == 0) return " fill=\"none\"";
    return " fill=\"" + c.css() + "\"";
}

const char* family_for(const Font& font) {
    const std::string& name = font.name();
    if (name.find("Bold") != std::string::npos) return "Liberation Sans, sans-serif";
    if (name.find("Italic") != std::string::npos) return "Liberation Sans, sans-serif";
    if (name.empty()) return "sans-serif";
    return "Liberation Sans, sans-serif";
}
}

SvgCanvas::SvgCanvas(int width, int height, Color background)
    : m_width(std::max(1, width)), m_height(std::max(1, height)) {
    if (background.a > 0)
        m_body += "<rect x=\"0\" y=\"0\" width=\"" + n(static_cast<float>(m_width)) + "\" height=\"" + n(static_cast<float>(m_height)) + "\" fill=\"" + background.css() + "\"/>\n";
}

void SvgCanvas::clear(Color c) {
    m_body += "<rect x=\"0\" y=\"0\" width=\"" + n(static_cast<float>(m_width)) + "\" height=\"" + n(static_cast<float>(m_height)) + "\" fill=\"" + c.css() + "\"/>\n";
}

void SvgCanvas::draw_line(Vec2 a, Vec2 b, Color c, float thickness) {
    if (c.a == 0) return;
    m_body += "<line x1=\"" + n(a.x) + "\" y1=\"" + n(a.y) + "\" x2=\"" + n(b.x) + "\" y2=\"" + n(b.y) + "\"" + stroke_attr(c, thickness) + "/>\n";
}

void SvgCanvas::draw_polyline(const std::vector<Vec2>& points, Color c, float thickness) {
    if (points.size() < 2 || c.a == 0) return;
    m_body += "<polyline points=\"" + points_attr(points) + "\" fill=\"none\"" + stroke_attr(c, thickness) + "/>\n";
}

void SvgCanvas::draw_polygon(const std::vector<Vec2>& points, Color fill, Color stroke, float thickness) {
    if (points.size() < 3) return;
    m_body += "<polygon points=\"" + points_attr(points) + "\"" + fill_attr(fill) + stroke_attr(stroke, thickness) + "/>\n";
}

void SvgCanvas::draw_rect(float x, float y, float w, float h, Color fill, Color stroke, float thickness) {
    m_body += "<rect x=\"" + n(x) + "\" y=\"" + n(y) + "\" width=\"" + n(w) + "\" height=\"" + n(h) + "\"" + fill_attr(fill) + stroke_attr(stroke, thickness) + "/>\n";
}

void SvgCanvas::draw_rounded_rect(float x, float y, float w, float h, float radius, Color fill, Color stroke, float thickness) {
    float r = std::max(0.0f, std::min(radius, std::min(w, h) * 0.5f));
    m_body += "<rect x=\"" + n(x) + "\" y=\"" + n(y) + "\" width=\"" + n(w) + "\" height=\"" + n(h) + "\" rx=\"" + n(r) + "\" ry=\"" + n(r) + "\"" + fill_attr(fill) + stroke_attr(stroke, thickness) + "/>\n";
}

void SvgCanvas::draw_circle(Vec2 center, float radius, Color fill, Color stroke, float thickness) {
    if (radius <= 0.0f) return;
    m_body += "<circle cx=\"" + n(center.x) + "\" cy=\"" + n(center.y) + "\" r=\"" + n(radius) + "\"" + fill_attr(fill) + stroke_attr(stroke, thickness) + "/>\n";
}

void SvgCanvas::draw_ellipse(Vec2 center, float rx, float ry, Color fill, Color stroke, float thickness) {
    if (rx <= 0.0f || ry <= 0.0f) return;
    m_body += "<ellipse cx=\"" + n(center.x) + "\" cy=\"" + n(center.y) + "\" rx=\"" + n(rx) + "\" ry=\"" + n(ry) + "\"" + fill_attr(fill) + stroke_attr(stroke, thickness) + "/>\n";
}

void SvgCanvas::draw_arc(Vec2 center, float radius, float start_rad, float end_rad, Color stroke, float thickness) {
    if (radius <= 0.0f || stroke.a == 0) return;
    float span = end_rad - start_rad;
    if (std::fabs(span) < 1e-6f) return;
    float x0 = center.x + radius * std::cos(start_rad);
    float y0 = center.y + radius * std::sin(start_rad);
    float x1 = center.x + radius * std::cos(end_rad);
    float y1 = center.y + radius * std::sin(end_rad);
    int large = std::fabs(span) > 3.14159265f ? 1 : 0;
    int sweep = span > 0.0f ? 1 : 0;
    m_body += "<path d=\"M " + n(x0) + " " + n(y0) + " A " + n(radius) + " " + n(radius) + " 0 " + std::to_string(large) + " " + std::to_string(sweep) + " " + n(x1) + " " + n(y1) + "\" fill=\"none\"" + stroke_attr(stroke, thickness) + "/>\n";
}

void SvgCanvas::draw_text(Vec2 position, const std::string& text, const Font& font, float px, Color c, TextAlign align, Anchor anchor, float rotation_rad) {
    if (text.empty() || c.a == 0) return;
    float asc = font.ascent(px);
    float desc = font.descent(px);
    float y = position.y;
    switch (anchor) {
        case Anchor::Middle: y += (asc - desc) * 0.5f; break;
        case Anchor::Top: y += asc; break;
        case Anchor::Bottom: y -= desc; break;
        case Anchor::Baseline: break;
    }
    const char* anchor_str = align == TextAlign::Center ? "middle" : (align == TextAlign::Right ? "end" : "start");
    m_body += "<text x=\"" + n(position.x) + "\" y=\"" + n(y) + "\" font-family=\"" + family_for(font) + "\" font-size=\"" + n(px) + "\" fill=\"" + c.css() + "\" text-anchor=\"" + anchor_str + "\" dominant-baseline=\"alphabetic\"";
    if (std::fabs(rotation_rad) > 1e-6f) m_body += " transform=\"rotate(" + n(rotation_rad * 180.0f / 3.14159265f) + " " + n(position.x) + " " + n(y) + ")\"";
    m_body += ">" + esc(text) + "</text>\n";
}

Vec2 SvgCanvas::measure_text(const std::string& text, const Font& font, float px) const {
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

std::string SvgCanvas::str() const {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + std::string("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"") + n(static_cast<float>(m_width)) + "\" height=\"" + n(static_cast<float>(m_height)) +
           "\" viewBox=\"0 0 " + n(static_cast<float>(m_width)) + " " + n(static_cast<float>(m_height)) + "\">\n" + m_body + "</svg>\n";
}

bool SvgCanvas::save(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out << str();
    return static_cast<bool>(out);
}
}
}
