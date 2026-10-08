#include "uranium/viz/theme.hpp"
namespace uranium {
namespace viz {
Theme Theme::light() { return Theme{}; }
Theme Theme::dark() {
    Theme t;
    t.background = Color{34, 37, 41};
    t.plot_background = Color{43, 47, 52};
    t.axis_color = Color{195, 198, 201};
    t.text_color = Color{230, 231, 233};
    t.grid_color = Color{72, 76, 81};
    t.legend_background = Color{54, 58, 64, 245};
    t.legend_border = Color{88, 92, 98};
    return t;
}
}
}
