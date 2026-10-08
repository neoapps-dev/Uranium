#include "uranium/viz/viz.hpp"
#include <cmath>
#include <cstdio>
#include <vector>
using namespace uranium;
using namespace uranium::viz;
int main() {
    std::vector<float> xs, sin_x, cos_x, band_hi, band_lo;
    for (int i = 0; i <= 300; ++i) {
        float x = i * 0.05f;
        xs.push_back(x);
        sin_x.push_back(std::sin(x));
        cos_x.push_back(std::cos(x));
        band_hi.push_back(0.4f * std::sin(x) + 0.3f);
        band_lo.push_back(0.4f * std::sin(x) - 0.3f);
    }

    Figure fig(1100, 750);
    fig.title("uranium :: viz showcase");
    Axes& a0 = fig.subplot(2, 2, 0);
    a0.plot(xs, sin_x, "sin x").line_width = 2.5f;
    a0.plot(xs, cos_x, "cos x").line_width = 2.0f;
    a0.fill_between(xs, band_hi, band_lo, "envelope");
    a0.axhline(0.0f);
    a0.title("lines + fill_between").xlabel("x").ylabel("y");
    a0.legend(LegendLoc::UpperRight);
    Axes& a1 = fig.subplot(2, 2, 1);
    std::vector<float> sx, sy;
    for (int i = 0; i < 60; ++i) {
        float u = i * 0.17f;
        sx.push_back(u);
        sy.push_back(std::sin(u) * (1.0f + u * 0.12f) + ((i % 5) - 2) * 0.06f);
    }
    Series& sc = a1.scatter(sx, sy, "samples");
    sc.marker = Marker::Diamond;
    sc.marker_size = 3.5f;
    a1.plot(sx, sy, "trend");
    a1.title("scatter + line").xlabel("t").ylabel("signal");
    a1.legend(LegendLoc::UpperLeft);
    Axes& a2 = fig.subplot(2, 2, 2);
    a2.bars({1, 2, 3, 4, 5}, {4, 7, 3, 9, 6}, "counts");
    a2.axhline(5.0f);
    a2.title("bars").xlabel("bin").ylabel("n");
    a2.grid(true);
    a2.legend(LegendLoc::UpperRight);
    Axes& a3 = fig.subplot(2, 2, 3);
    Series& l1 = a3.plot(xs, sin_x, "damped");
    l1.line_width = 1.5f;
    for (auto& pt : l1.points) pt.y *= std::exp(-0.08f * pt.x);
    Series& l2 = a3.plot(xs, cos_x, "damped quad");
    l2.marker = Marker::Circle;
    l2.marker_size = 2.0f;
    for (auto& pt : l2.points) {
        pt.y *= std::exp(-0.05f * pt.x);
    }
    a3.title("decay").xlabel("t").ylabel("amplitude");
    a3.annotate("dying oscillation", {10.0f, 0.0f}, {40.0f, 45.0f});
    a3.legend(LegendLoc::UpperRight);
    a3.spines(Spines::Minimal);
    bool ok1 = fig.save_png("showcase.png", 1.5f);
    bool ok2 = fig.save_svg("showcase.svg");
    std::printf("showcase.png=%d showcase.svg=%d\n", (int)ok1, (int)ok2);
    Figure dark(900, 500);
    dark.set_theme(Theme::dark());
    dark.title("dark theme");
    Axes& d0 = dark.axes();
    d0.plot(xs, sin_x, "sin").line_width = 2.5f;
    d0.fill_between(xs, band_hi, band_lo, "band");
    d0.scatter(sx, sy, "samples");
    d0.title("everything at once").xlabel("x").ylabel("y");
    d0.legend(LegendLoc::LowerLeft);
    bool ok3 = dark.save_png("showcase_dark.png");
    std::printf("showcase_dark.png=%d\n", (int)ok3);
    return (ok1 && ok2 && ok3) ? 0 : 1;
}
