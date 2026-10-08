#include "uranium/equation/equation.hpp"
#include "uranium/render/render.hpp"
#include <cstdio>
#include <vector>
using namespace uranium;
using namespace uranium::equation;
using namespace uranium::render;
int main() {
    const std::vector<std::string> formulas = {
        "E = mc^2",
        "\\frac{-b \\pm \\sqrt{b^2 - 4ac}}{2a}",
        "\\sum_{i=1}^{n} i = \\frac{n(n+1)}{2}",
        "\\int_0^\\infty e^{-x^2}\\,dx = \\frac{\\sqrt{\\pi}}{2}",
        "\\nabla \\times \\vec{B} = \\mu_0 \\vec{J}",
        "\\lim_{x \\to 0} \\frac{\\sin x}{x} = 1",
        "\\hat{\\psi}(x) = \\sum_{n} c_n \\phi_n(x)",
        "\\sqrt[3]{x+1} + \\log_2 8 = \\alpha + \\beta",
        "\\mathbf{F} = m\\,\\mathbf{a}, \\qquad \\text{valid for } m > 0",
        "e^{i\\pi} + 1 = 0",
    };

    RasterCanvas canvas(1000, 640, colors::white);
    canvas.draw_rect(0, 0, 1000, 640, Color{252, 252, 250});
    canvas.draw_text({500.0f, 34.0f}, "uranium :: equation renderer", FontBook::instance().default_font(), 26.0f, render::Color::gray(40), TextAlign::Center, Anchor::Middle);
    float y = 90.0f;
    for (const std::string& tex : formulas) {
        Equation eq(tex);
        if (!eq.valid()) {
            std::printf("parse error: %s\n", eq.error().c_str());
            return 1;
        }
        eq.draw(canvas, {70.0f, y}, 30.0f, render::Color::gray(20));
        y += 54.0f;
    }

    Equation centered("\\frac{\\partial u}{\\partial t} = \\alpha \\nabla^2 u");
    Layout m = centered.measure(34.0f);
    centered.draw_aligned(canvas, {500.0f, 610.0f}, 34.0f, colors::blue, TextAlign::Center);
    std::printf("centered measure: %.1f x %.1f (asc %.1f, desc %.1f)\n", m.width, m.height(), m.ascent, m.descent);
    Equation bad("\\frac{1}");
    std::printf("invalid example detected: %d (%s)\n", (int)(!bad.valid()), bad.error().c_str());
    bool ok = canvas.save_png("equations.png");
    SvgCanvas svg(1000, 640, colors::white);
    for (std::size_t i = 0; i < formulas.size(); ++i) Equation(formulas[i]).draw(svg, {70.0f, 90.0f + 54.0f * static_cast<float>(i)}, 30.0f, colors::black);
    bool ok2 = svg.save("equations.svg");
    std::printf("equations.png=%d equations.svg=%d\n", (int)ok, (int)ok2);
    return (ok && ok2) ? 0 : 1;
}
