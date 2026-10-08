#pragma once
#include "uranium/core/config.hpp"
#include "uranium/render/color.hpp"
namespace uranium {
namespace viz {
using render::Color;
struct URANIUM_API Theme {
    Color background = Color{255, 255, 255};
    Color plot_background = Color{255, 255, 255};
    Color axis_color = Color{70, 70, 70};
    Color text_color = Color{45, 45, 45};
    Color grid_color = Color{214, 214, 214};
    Color legend_background = Color{252, 252, 250, 245};
    Color legend_border = Color{195, 195, 195};
    static Theme light();
    static Theme dark();
};

enum class LegendLoc { UpperRight = 0, UpperLeft = 1, LowerRight = 2, LowerLeft = 3 };
enum class Spines { Box, Minimal };
}
}
