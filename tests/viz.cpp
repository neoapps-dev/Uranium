#include "check.hpp"
#include "uranium/core/error.hpp"
#include "uranium/render/render.hpp"
#include "uranium/viz/viz.hpp"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
using namespace uranium;
using namespace uranium::viz;
static int count_exact(render::RasterCanvas& c, render::Color target, int x0, int y0, int x1,
                       int y1) {
    int n = 0;
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x)
            if (c.at(x, y) == target) ++n;
    return n;
}

static unsigned png_width(const char* path) {
    std::FILE* f = std::fopen(path, "rb");
    if (!f) return 0;
    unsigned char hdr[24];
    std::size_t got = std::fread(hdr, 1, 24, f);
    std::fclose(f);
    if (got != 24) return 0;
    return (static_cast<unsigned>(hdr[16]) << 24) | (static_cast<unsigned>(hdr[17]) << 16) |
           (static_cast<unsigned>(hdr[18]) << 8) | static_cast<unsigned>(hdr[19]);
}

int main() {
    Figure fig(800, 500);
    fig.title("viz test");
    Axes& a0 = fig.subplot(2, 1, 0);
    std::vector<float> xs, ys;
    for (int i = 0; i <= 200; ++i) {
        xs.push_back(i * 0.05f);
        ys.push_back(std::sin(i * 0.05f));
    }
    a0.plot(xs, ys, "sin");
    a0.title("trig").xlabel("x").ylabel("f(x)");
    a0.legend();
    Axes& a1 = fig.subplot(2, 1, 1);
    a1.bars({1, 2, 3, 4}, {3, 5, 2, 4}, "count");
    a1.scatter({1.5f, 2.5f, 3.5f}, {4.0f, 3.0f, 5.0f}, "obs");
    a1.axhline(3.0f);
    a1.annotate("peak", {2, 5}, {0, -25});
    a1.title("bars");
    a1.legend(LegendLoc::UpperLeft);
    render::RasterCanvas raster(800, 500, render::colors::white);
    fig.render(raster);
    const render::Rect& R0 = a0.plot_rect();
    const render::Rect& R1 = a1.plot_rect();
    CHECK(R0.w > 100.0f && R0.h > 60.0f);
    CHECK(R1.w > 100.0f && R1.h > 60.0f);
    CHECK(a0.y_range().lo < 0.0f);
    CHECK(a0.y_range().hi > 1.0f);
    CHECK(a1.x_range().lo < 1.0f);
    CHECK(a1.x_range().hi > 4.0f);
    int blue = count_exact(raster, render::Color{38, 139, 210}, static_cast<int>(R0.x),
                           static_cast<int>(R0.y), static_cast<int>(R0.right()),
                           static_cast<int>(R0.bottom()));
    CHECK(blue > 50);
    int red = count_exact(raster, render::Color{220, 50, 47}, static_cast<int>(R1.x),
                          static_cast<int>(R1.y), static_cast<int>(R1.right()),
                          static_cast<int>(R1.bottom()));
    CHECK(red > 50);
    int plot_bg = count_exact(raster, render::colors::white, static_cast<int>(R0.x) + 4,
                              static_cast<int>(R0.y) + 4, static_cast<int>(R0.x) + 40,
                              static_cast<int>(R0.y) + 40);
    CHECK(plot_bg > 0);
    Vec2 rt = a0.to_data(a0.to_pixel({1.0f, 0.5f}));
    CHECK_NEAR(rt.x, 1.0f, 1e-3f);
    CHECK_NEAR(rt.y, 0.5f, 1e-3f);
    CHECK(fig.save_png("test_viz_out.png"));
    CHECK(fig.save_png("test_viz_out2x.png", 2.0f));
    CHECK(fig.save_svg("test_viz_out.svg"));
    CHECK(png_width("test_viz_out.png") == 800);
    CHECK(png_width("test_viz_out2x.png") == 1600);
    std::FILE* f = std::fopen("test_viz_out.svg", "rb");
    CHECK(f != nullptr);
    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    std::fclose(f);
    CHECK(sz > 5000);
    Figure dark(300, 200);
    dark.set_theme(Theme::dark());
    Axes& d = dark.axes();
    d.plot({0, 1, 2, 3}, {1, 3, 2, 4});
    render::RasterCanvas dr(300, 200, render::colors::white);
    dark.render(dr);
    render::Color corner = dr.at(1, 1);
    CHECK((corner == render::Color{34, 37, 41}));
    const render::Rect& DR = d.plot_rect();
    render::Color pbg = dr.at(static_cast<int>(DR.x + 5), static_cast<int>(DR.y + 5));
    CHECK((pbg == render::Color{43, 47, 52}));
    CHECK(dark.save_png("test_viz_dark.png"));
    Figure one(400, 300);
    Axes& oa = one.axes();
    CHECK(&oa == &one.axes());
    oa.fill_between({0, 1, 2, 3}, {2, 3, 3, 2}, {0, 1, 1, 0}, "band");
    oa.set_xlim(-1.0f, 4.0f);
    oa.set_ylim(-1.0f, 4.0f);
    render::RasterCanvas orr(400, 300, render::colors::white);
    one.render(orr);
    const render::Rect& OR = oa.plot_rect();
    int band = 0;
    for (int y = static_cast<int>(OR.y) + 3; y < static_cast<int>(OR.bottom()) - 3; ++y)
        for (int x = static_cast<int>(OR.x) + 3; x < static_cast<int>(OR.right()) - 3; ++x) {
            render::Color p = orr.at(x, y);
            if (p.r < 245 && p.b > 170 && p.g > 140) ++band;
        }
    CHECK(band > 200);
    Figure sub(600, 400);
    Axes& s0 = sub.subplot(2, 2, 0);
    Axes& s3 = sub.subplot(2, 2, 3);
    s0.plot({0, 1}, {0, 1});
    s3.plot({0, 1}, {1, 0});
    render::RasterCanvas sr(600, 400, render::colors::white);
    sub.render(sr);
    CHECK(s0.plot_rect().x < s3.plot_rect().x);
    CHECK(s0.plot_rect().y < s3.plot_rect().y);
    CHECK(s0.plot_rect().right() <= s3.plot_rect().x + 1.0f);
    bool caught = false;
    try {
        sub.subplot(2, 2, 9);
    } catch (const ValueError&) {
        caught = true;
    }
    CHECK(caught);
    return 0;
}
