#pragma once
#include "uranium/core/config.hpp"
#include "uranium/math/vector.hpp"
#include "uranium/render/color.hpp"
#include <string>
#include <vector>
namespace uranium {
namespace viz {
using render::Color;
enum class SeriesKind { Line, Scatter, Bars, Fill };
enum class Marker { Circle, Square, Cross, Plus, Diamond, Triangle };
struct URANIUM_API Series {
    SeriesKind kind = SeriesKind::Line;
    std::vector<Vec2> points;
    std::vector<Vec2> baseline_points;
    float bar_width = 0.0f;
    std::string label;
    Color color = Color{0, 0, 0, 0};
    float line_width = 2.0f;
    float marker_size = 4.0f;
    Marker marker = Marker::Circle;
    bool show_in_legend = true;
};
}
}
