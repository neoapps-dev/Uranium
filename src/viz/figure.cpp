#include "uranium/viz/figure.hpp"
#include "uranium/core/error.hpp"
#include "uranium/render/font.hpp"
#include "uranium/render/raster.hpp"
#include "uranium/render/svg.hpp"
#include "uranium/render/transform.hpp"
#include <algorithm>
#include <cmath>
namespace uranium {
namespace viz {
namespace {
float compute_base(float w, float h) {
    float b = std::min(w, h) / 40.0f;
    return std::max(9.0f, std::min(18.0f, b));
}
}

Figure::Figure(float width, float height): m_width(width > 1.0f ? width : 1.0f), m_height(height > 1.0f ? height : 1.0f), m_theme(Theme::light()) {}
Axes& Figure::axes() {
    if (m_panels.empty()) return subplot(1, 1, 0);
    return m_panels[0];
}

Axes& Figure::subplot(std::size_t rows, std::size_t cols, std::size_t index) {
    if (rows == 0 || cols == 0) throw ValueError("subplot: rows and cols must be >= 1");
    if (index >= rows * cols) throw ValueError("subplot index out of range");
    m_rows = rows;
    m_cols = cols;
    while (m_panels.size() <= index) m_panels.emplace_back();
    return m_panels[index];
}

Axes& Figure::panel(std::size_t index) {
    if (index >= m_panels.size()) throw ValueError("panel index out of range");
    return m_panels[index];
}

Figure& Figure::title(std::string text) {
    m_title = std::move(text);
    return *this;
}

Figure& Figure::set_theme(Theme theme) {
    m_theme = theme;
    return *this;
}

Figure& Figure::set_size(float width, float height) {
    m_width = width > 1.0f ? width : 1.0f;
    m_height = height > 1.0f ? height : 1.0f;
    return *this;
}

void Figure::render_panels(render::Canvas& canvas) {
    if (m_panels.empty()) return;
    float base = compute_base(m_width, m_height);
    float outer_l = base * 0.5f;
    float outer_r = base * 0.5f;
    float outer_b = base * 0.5f;
    float outer_t = base * 0.6f + (m_title.empty() ? 0.0f : base * 1.8f);
    if (!m_title.empty()) canvas.draw_text({m_width * 0.5f, outer_t * 0.5f}, m_title, render::FontBook::instance().default_font(), base * 1.15f, m_theme.text_color, render::TextAlign::Center, render::Anchor::Middle);
    float avail_w = std::max(40.0f, m_width - outer_l - outer_r);
    float avail_h = std::max(40.0f, m_height - outer_t - outer_b);
    float gap_w = m_cols > 1 ? (avail_w / static_cast<float>(m_cols)) * 0.14f : 0.0f;
    float gap_h = m_rows > 1 ? (avail_h / static_cast<float>(m_rows)) * 0.18f : 0.0f;
    float cell_w = (avail_w - gap_w * static_cast<float>(m_cols - 1)) / static_cast<float>(m_cols);
    float cell_h = (avail_h - gap_h * static_cast<float>(m_rows - 1)) / static_cast<float>(m_rows);
    std::size_t visible = std::min(m_panels.size(), m_rows * m_cols);
    for (std::size_t i = 0; i < visible; ++i) {
        std::size_t row = i / m_cols;
        std::size_t col = i % m_cols;
        Rect cell(outer_l + static_cast<float>(col) * (cell_w + gap_w), outer_t + static_cast<float>(row) * (cell_h + gap_h), cell_w, cell_h);
        Axes& ax = m_panels[i];
        ax.autoscale();
        Margins m = ax.measure(base, cell);
        float w = std::max(30.0f, cell.w - m.left - m.right);
        float h = std::max(30.0f, cell.h - m.top - m.bottom);
        ax.set_plot_rect(Rect(cell.x + m.left, cell.y + m.top, w, h));
        ax.render(canvas, m_theme, base, cell);
    }
}

void Figure::render(render::Canvas& device) {
    float s = std::min(static_cast<float>(device.width()) / m_width, static_cast<float>(device.height()) / m_height);
    if (!(s > 0.0f)) s = 1.0f;
    render::Vec2 offset{(static_cast<float>(device.width()) - m_width * s) * 0.5f, (static_cast<float>(device.height()) - m_height * s) * 0.5f};
    device.clear(m_theme.background);
    render::TransformCanvas tc(device, s, offset);
    render_panels(tc);
}

bool Figure::save_png(const std::string& path, float scale) {
    if (!(scale > 0.0f)) scale = 1.0f;
    int w = std::max(1, static_cast<int>(std::lround(m_width * scale)));
    int h = std::max(1, static_cast<int>(std::lround(m_height * scale)));
    render::RasterCanvas raster(w, h, m_theme.background);
    render(raster);
    return raster.save_png(path);
}

bool Figure::save_svg(const std::string& path) {
    render::SvgCanvas svg(static_cast<int>(std::lround(m_width)), static_cast<int>(std::lround(m_height)), m_theme.background);
    render(svg);
    return svg.save(path);
}
}
}
