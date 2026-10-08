#include "check.hpp"
#include "uranium/render/render.hpp"
#include <cstdio>
#include <string>
#include <vector>
using namespace uranium;
using namespace uranium::render;
int main() {
    CHECK((Color{255, 0, 0}.mixed(Color{0, 0, 255}, 0.5f).r > 120));
    CHECK(Color::from_hex(0x336699).b == 0x99);
    CHECK(colors::palette(0) == colors::palette(10));
    RasterCanvas rc(64, 48, colors::white);
    rc.clear(colors::white);
    CHECK(rc.at(0, 0) == colors::white);
    rc.draw_rect(10.0f, 10.0f, 20.0f, 20.0f, Color{255, 0, 0});
    CHECK(rc.at(20, 20) == (Color{255, 0, 0}));
    CHECK(rc.at(5, 5) == colors::white);
    CHECK(rc.at(31, 20) == colors::white);
    rc.draw_circle({50.0f, 30.0f}, 6.0f, Color{0, 255, 0});
    CHECK(rc.at(50, 30) == (Color{0, 255, 0}));
    RasterCanvas line_canvas(100, 100, colors::white);
    line_canvas.draw_line({10, 10}, {90, 90}, colors::black, 2.0f);
    int dark = 0;
    for (int y = 0; y < 100; ++y)
        for (int x = 0; x < 100; ++x)
            if (line_canvas.at(x, y).r < 128) ++dark;
    CHECK(dark > 100);
    line_canvas.draw_line({5, 95}, {95, 5}, colors::red, 3.0f);
    int red = 0;
    for (int y = 0; y < 100; ++y)
        for (int x = 0; x < 100; ++x)
            if (line_canvas.at(x, y) == colors::red) ++red;
    CHECK(red > 200);
    const Font& font = FontBook::instance().default_font();
    CHECK(font.loaded());
    CHECK(font.advance("Hi", 20.0f) > 0.0f);
    CHECK(font.advance("Hi", 20.0f) < font.advance("HiHi", 20.0f));
    CHECK(font.ascent(16.0f) > 0.0f);
    CHECK(!FontBook::instance().names().empty());
    RasterCanvas text_canvas(200, 60, colors::white);
    text_canvas.draw_text({10.0f, 35.0f}, "Hello Uranium", font, 24.0f, colors::black);
    int ink = 0;
    for (int y = 0; y < 60; ++y)
        for (int x = 0; x < 200; ++x)
            if (text_canvas.at(x, y).r < 200) ++ink;
    CHECK(ink > 100);
    RasterCanvas kern_canvas(200, 60, colors::white);
    kern_canvas.draw_text({10.0f, 35.0f}, "AV", font, 40.0f, colors::black);
    (void)font.kerning('A', 'V', 40.0f);
    SvgCanvas svg(200, 100, colors::white);
    svg.draw_line({0, 0}, {200, 100}, colors::blue, 2.0f);
    svg.draw_rect(5, 5, 50, 30, colors::red.with_alpha(128));
    svg.draw_text({20, 70}, "x < 3 & y > 1", font, 16.0f, colors::black);
    std::string s = svg.str();
    CHECK(s.find("<line") != std::string::npos);
    CHECK(s.find("<rect") != std::string::npos);
    CHECK(s.find("<text") != std::string::npos);
    CHECK(s.find("&lt;") != std::string::npos);
    CHECK(s.find("&amp;") != std::string::npos);
    SvgCanvas svg2(64, 48, colors::white);
    RasterCanvas tiny(64, 48, colors::white);
    TransformCanvas tc(tiny, 2.0f, Vec2{10.0f, 10.0f});
    tc.draw_rect(0, 0, 5, 5, Color{0, 0, 255});
    CHECK(tiny.at(12, 12) == (Color{0, 0, 255}));
    CHECK(tiny.at(0, 0) == colors::white);
    (void)svg2;
    RasterCanvas dash_canvas(80, 80, colors::white);
    dash_canvas.draw_dashed_line({10, 40}, {70, 40}, colors::black, 2.0f, 4.0f, 4.0f);
    int dash_dark = 0;
    int total = 0;
    for (int y = 38; y < 42; ++y)
        for (int x = 0; x < 80; ++x) {
            ++total;
            if (dash_canvas.at(x, y).r < 128) ++dash_dark;
        }
    CHECK(dash_dark > 0);
    CHECK(dash_dark < total);
    CHECK(rc.save_png("test_render_out.png"));
    CHECK(svg.save("test_render_out.svg"));
    FILE* f = std::fopen("test_render_out.png", "rb");
    CHECK(f != nullptr);
    unsigned char hdr[8];
    CHECK(std::fread(hdr, 1, 8, f) == 8);
    static const unsigned char sig[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n'};
    for (int i = 0; i < 8; ++i) CHECK(hdr[i] == sig[i]);
    std::fclose(f);
    return 0;
}
