#pragma once
#include "uranium/core/config.hpp"
#include "uranium/render/canvas.hpp"
#include "uranium/viz/series.hpp"
#include "uranium/viz/theme.hpp"
#include <string>
#include <vector>
namespace uranium {
namespace viz {
using render::Rect;
using render::TextAlign;
struct URANIUM_API Range {
    float lo = 0.0f;
    float hi = 1.0f;
    URANIUM_NODISCARD float span() const { return hi - lo; }
    URANIUM_NODISCARD bool contains(float v) const { return v >= lo && v <= hi; }
    URANIUM_NODISCARD float clamped(float v) const {
        return v < lo ? lo : (v > hi ? hi : v);
    }
};

struct URANIUM_API Margins {
    float left = 40.0f;
    float right = 12.0f;
    float top = 16.0f;
    float bottom = 30.0f;
};

struct URANIUM_API Annotation {
    std::string text;
    Vec2 data;
    Vec2 offset{0.0f, -22.0f};
    TextAlign align = TextAlign::Center;
    bool show_point = true;
    Color color = Color{0, 0, 0, 0};
};

class URANIUM_API Axes {
public:
    Axes();
    Series& plot(const std::vector<float>& xs, const std::vector<float>& ys, std::string label = "");
    Series& plot(const std::vector<Vec2>& points, std::string label = "");
    Series& scatter(const std::vector<float>& xs, const std::vector<float>& ys, std::string label = "");
    Series& scatter(const std::vector<Vec2>& points, std::string label = "");
    Series& bars(const std::vector<float>& xs, const std::vector<float>& heights, std::string label = "", float width = 0.0f);
    Series& fill_between(const std::vector<float>& xs, const std::vector<float>& y_upper, const std::vector<float>& y_lower, std::string label = "");
    Series& add(Series series);
    Axes& axhline(float y, Color color = Color{0, 0, 0, 0}, float thickness = 1.5f);
    Axes& axvline(float x, Color color = Color{0, 0, 0, 0}, float thickness = 1.5f);
    Axes& title(std::string text);
    Axes& xlabel(std::string text);
    Axes& ylabel(std::string text);
    Axes& set_xlim(float lo, float hi);
    Axes& set_ylim(float lo, float hi);
    Axes& set_range(float x_lo, float x_hi, float y_lo, float y_hi);
    Axes& legend(bool show = true);
    Axes& legend(LegendLoc loc);
    Axes& grid(bool show = true);
    Axes& spines(Spines style);
    Axes& annotate(std::string text, Vec2 data, Vec2 offset = Vec2{0.0f, -22.0f}, TextAlign align = TextAlign::Center);
    URANIUM_NODISCARD const Range& x_range() const { return m_xrange; }
    URANIUM_NODISCARD const Range& y_range() const { return m_yrange; }
    URANIUM_NODISCARD const Rect& plot_rect() const { return m_plot_rect; }
    URANIUM_NODISCARD const std::vector<Series>& series() const { return m_series; }
    URANIUM_NODISCARD LegendLoc legend_loc() const { return m_legend_loc; }
    URANIUM_NODISCARD Vec2 to_pixel(Vec2 data) const;
    URANIUM_NODISCARD Vec2 to_data(Vec2 pixel) const;
    void set_plot_rect(Rect rect) { m_plot_rect = rect; }
    void autoscale();
    Margins measure(float base_px, const Rect& cell) const;
    void render(render::Canvas& canvas, const Theme& theme, float base_px, const Rect& cell) const;
private:
    struct GuideLine {
        bool vertical = false;
        float value = 0.0f;
        Color color = Color{0, 0, 0, 0};
        float thickness = 1.5f;
    };

    struct TickInfo {
        std::vector<float> values;
        std::vector<std::string> labels;
        float step = 1.0f;
    };

    TickInfo x_ticks(const Rect& cell) const;
    TickInfo y_ticks(const Rect& cell) const;
    std::vector<Series> m_series;
    std::vector<GuideLine> m_guides;
    std::vector<Annotation> m_annotations;
    Range m_xrange;
    Range m_yrange;
    Rect m_plot_rect;
    std::string m_title;
    std::string m_xlabel;
    std::string m_ylabel;
    bool m_legend = false;
    LegendLoc m_legend_loc = LegendLoc::UpperRight;
    bool m_grid = true;
    Spines m_spines = Spines::Box;
    bool m_explicit_x = false;
    bool m_explicit_y = false;
};
}
}
