#include "check.hpp"
#include "uranium/core/error.hpp"
#include "uranium/equation/equation.hpp"
#include "uranium/render/render.hpp"
using namespace uranium;
using namespace uranium::equation;
using namespace uranium::render;
static int ink_in(RasterCanvas& c, int x0, int y0, int x1, int y1) {
    int n = 0;
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x) {
            Color p = c.at(x, y);
            if (p.r < 240 || p.g < 240 || p.b < 240) ++n;
        }
    return n;
}

int main() {
    Equation good("E = mc^2");
    CHECK(good.valid());
    Layout m = good.measure(30.0f);
    CHECK(m.width > 30.0f);
    CHECK(m.ascent > 10.0f);
    CHECK(m.descent >= 0.0f);
    Equation frac("\\frac{-b \\pm \\sqrt{b^2 - 4ac}}{2a}");
    CHECK(frac.valid());
    Layout mf = frac.measure(30.0f);
    CHECK(mf.ascent > m.ascent);
    CHECK(mf.width > 60.0f);
    CHECK(Equation("\\sqrt{x+1}").valid());
    CHECK(Equation("\\sum_{i=0}^{n} i^2").valid());
    CHECK(Equation("\\int_0^\\infty e^{-x^2} dx").valid());
    CHECK(Equation("\\alpha + \\beta = \\gamma").valid());
    CHECK(Equation("\\hat{\\psi} + \\bar{x} + \\vec{v} + \\dot{q}").valid());
    CHECK(Equation("\\lim_{x \\to 0} \\frac{\\sin x}{x} = 1").valid());
    CHECK(Equation("\\mathbf{F} = m\\,\\mathbf{a}").valid());
    CHECK(Equation("\\text{hello world}").valid());
    CHECK(Equation("\\sqrt[3]{8}").valid());
    CHECK(!Equation("\\frac{1}").valid());
    CHECK(!Equation("\\notacommand{x}").valid());
    CHECK(!Equation("\\frac{a").valid());
    Equation threw("\\bogus");
    bool caught = false;
    try {
        RasterCanvas scratch(10, 10);
        draw(scratch, {0, 0}, "\\bogus", 10.0f);
    } catch (const ParseError&) {
        caught = true;
    }
    CHECK(caught);
    RasterCanvas canvas(600, 400, colors::white);
    Equation eq("\\frac{a}{b} + \\sqrt{c^2} = \\sum_{k=1}^{n} k");
    eq.draw(canvas, {40.0f, 80.0f}, 34.0f, colors::black);
    int ink1 = ink_in(canvas, 0, 0, 600, 120);
    CHECK(ink1 > 300);
    Equation eq2("\\nabla \\times \\vec{E} = -\\frac{\\partial \\vec{B}}{\\partial t}");
    eq2.draw_aligned(canvas, {300.0f, 220.0f}, 28.0f, colors::blue, TextAlign::Center);
    int ink2 = ink_in(canvas, 0, 150, 600, 260);
    CHECK(ink2 > 300);
    Equation eq3("\\pi r^2 \\approx 3.14159");
    eq3.draw(canvas, {40.0f, 340.0f}, 30.0f, colors::red);
    int ink3 = ink_in(canvas, 0, 280, 600, 380);
    CHECK(ink3 > 200);
    CHECK(canvas.save_png("test_equation_out.png"));
    return 0;
}
