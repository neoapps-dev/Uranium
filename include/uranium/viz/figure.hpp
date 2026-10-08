#pragma once
#include "uranium/core/config.hpp"
#include "uranium/render/canvas.hpp"
#include "uranium/viz/axes.hpp"
#include "uranium/viz/theme.hpp"
#include <cstddef>
#include <deque>
#include <string>
namespace uranium {
namespace viz {
class URANIUM_API Figure {
public:
    explicit Figure(float width = 900.0f, float height = 600.0f);
    Axes& axes();
    Axes& subplot(std::size_t rows, std::size_t cols, std::size_t index);
    Axes& panel(std::size_t index);
    URANIUM_NODISCARD std::size_t panel_count() const { return m_panels.size(); }
    URANIUM_NODISCARD float width() const { return m_width; }
    URANIUM_NODISCARD float height() const { return m_height; }
    Figure& title(std::string text);
    Figure& set_theme(Theme theme);
    Figure& set_size(float width, float height);
    void render(render::Canvas& device);
    bool save_png(const std::string& path, float scale = 1.0f);
    bool save_svg(const std::string& path);

private:
    void render_panels(render::Canvas& canvas);
    float m_width;
    float m_height;
    std::deque<Axes> m_panels;
    std::size_t m_rows = 1;
    std::size_t m_cols = 1;
    std::string m_title;
    Theme m_theme;
};
}
}
