<div align="center">
<img src="https://raw.githubusercontent.com/neoapps-dev/Uranium/refs/heads/main/.github/assets/banner.png">
</div>

# Uranium

A modular C++17 library for mathematics, physics simulation, equation/plot rendering, statistics, and equation typesetting. Uranium is designed to be *ridiculously flexible* with a small, friendly surface area.

## Building

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

Options:

| Option | Default | Description |
| ------ | ------- | ----------- |
| `URANIUM_BUILD_SHARED` | `OFF` | Build compiled modules (viz, equation and render) as shared libraries |
| `URANIUM_BUILD_EXAMPLES` | `ON` | Build the examples in `examples/` |
| `URANIUM_BUILD_TESTS` | `ON` | Build and register the test suite |

Install and consume with CMake:

```sh
cmake --install build --prefix /usr/local
```

```cmake
find_package(uranium REQUIRED)
target_link_libraries(app PRIVATE uranium::uranium)
```

```cpp
#include <uranium/viz/viz.hpp>
```

## Namespaces

```cpp
using namespace uranium; // Vec3, Matrix, Quaternion, Mat3, colors
using namespace uranium::numerical; // fft, qr, rk45, simpson, ...
using namespace uranium::stats; // mean, median, stddev, fit_linear, ...
using namespace uranium::physics; // RigidBody, ParticleSystem, World, gravity, ...
using namespace uranium::render; // RasterCanvas, SvgCanvas, Color, Font, ...
using namespace uranium::viz; // Figure, Axes, Series, Theme, ...
using namespace uranium::equation; // Equation, ParseError
```

## Quick tour (hehehe you'll like it)

### Math

```cpp
#include <uranium/math/math.hpp>
using namespace uranium;
Vec3 a{1, 2, 3}, b{4, 5, 6};
float d = dot(a, b);
Vec3 c = cross(a, b);
Matrix<double, 3, 3> m = Matrix<double, 3, 3>::identity();
Quaternion<float> q = Quaternion<float>::from_axis_angle({0, 1, 0}, 0.5f);
Vec3 r = q.rotate(a);
```

### Numerical & statistics

```cpp
#include <uranium/math/math.hpp>
#include <uranium/math/numerical/numerical.hpp>
#include <uranium/math/stats/stats.hpp>
using namespace uranium;
using namespace uranium::numerical;
using namespace uranium::stats;
auto spectrum = fft(std::vector<std::complex<double>>{{1, 0}, {2, 0}, {3, 0}, {4, 0}});
double x = newton([](double v) { return v * v - 2.0; }, [](double v) { return 2.0 * v; }, 1.0);
auto qr = decompose_qr(matrix);
auto fit = fit_linear(xs, ys); //neo: fit.slope, fit.intercept
double s = stddev(values); //neo: sample stddev (ddof=1)
```

### Physics

```cpp
#include <uranium/physics/physics.hpp>
using namespace uranium;
using namespace uranium::physics;
ParticleSystem sys; //neo: ForceFields return force in Newtons, acceleration = F/m
sys.add(Particle::at({0, 10, 0}, 1.0)) // position{x,y,z}, mass
   .add_field(gravity() + linear_drag(0.1))
   .use(Method::RK4);
for (int i = 0; i < 600; ++i) sys.step(1.0 / 60.0);
World world;
world.add(Plane::at_point({0, 0, 0}, {0, 1, 0}));
RigidBody floor = RigidBody::solid_box({10, 0.5, 10}, 8000.0, {0, -0.5, 0});  //neo: half extents!
floor.make_static();
world.add(floor);
int ball = world.add(RigidBody::solid_sphere(0.5, 400.0, {0, 3, 0}));
world.body(ball).restitution = 0.7;
for (int i = 0; i < 600; ++i) world.step(1.0 / 60.0);
```

Note that `solid_box` takes **half extents**. `physics::Vec3` is `Vector<double, 3>`, shadowing `uranium::Vec3` (`Vector<float, 3>`); likewise physics declares its own double-precision `Mat3` and `Quat`. Prefer `physics::Vec3` in physics code.

### Render (renderer-agnostic)

```cpp
#include <uranium/render/render.hpp>
using namespace uranium;
using namespace uranium::render;
RasterCanvas canvas(800, 600, colors::white);
canvas.draw_line({10, 10}, {790, 10}, colors::red, 2.0f);
canvas.draw_circle({400, 300}, 120, colors::blue, colors::transparent, 3.0f);
canvas.draw_text({30, 30}, "hello", *FontBook::instance().default_font(), 24, colors::black);
canvas.save_png("out.png"); //neo: raster
canvas.save_svg("out.svg"); //neo: the SVG backend shares the same API
```

`render::Canvas` is an abstract interface; `RasterCanvas`, `SvgCanvas`, and `render::TransformCanvas` (an affine scale/offset adapter) all implement it, so the renderer you target is a construction-time choice.

### Viz

```cpp
#include <uranium/viz/viz.hpp>
using namespace uranium;
using namespace uranium::viz;
Figure fig(900, 600);
fig.title("Uranium");
Axes& a = fig.subplot(2, 1, 0);
a.plot(xs, ys, "line").scatter(xs2, ys2, "pts");
a.title("top").legend();
Axes& b = fig.subplot(2, 1, 1);
b.bars(labels, values, "bars").fill_between(xs, hi, lo, "band");
b.set_xlim(-1, 5).set_ylim(0, 10);
fig.set_theme(Theme::dark());
fig.save_png("figure.png", 1.5f); //neo: scale factor for hi-dpi output
fig.save_svg("figure.svg");
```

`Figure::subplot(rows, cols, index)` lays out cells; the grid spacing preferences in `Axes` can be overridden via `set_xlim`/`set_ylim`/`set_range`. Series are auto-colored from the palette when no explicit color is given. Everything renders through the `render::Canvas` API, so figures can be drawn into any backend.

### Equations

```cpp
#include <uranium/equation/equation.hpp>
using namespace uranium;
using namespace uranium::equation;
Equation eq("\\frac{-b \\pm \\sqrt{b^2 - 4ac}}{2a}");
if (!eq.valid()) throw eq.error();
eq.draw(canvas, {40, 40}, 36.0f, colors::black); //neo: top-left anchored
eq.draw_aligned(canvas, {400, 300}, 30.0f, colors::blue, TextAlign::Center, Anchor::Middle);
```

Supported syntax includes fractions, roots (with optional degree), sums, integrals, limits, accents (`\hat`, `\bar`, `\vec`, `\dot`), bold/italic text, Greek letters, text, sub/superscripts, and grouping with `{}`. Parsing errors raise `ParseError` (try `Equation::error()` for a message).

## Fonts

Text rendering uses TTF fonts via a vendored `stb_truetype`. At runtime the font book searches, in order:
1. the `URANIUM_FONT_DIR` environment variable,
2. the build-tree font directory baked in at compile time,
3. the install directory `share/uranium/fonts`,
4. a `assets/fonts` directory relative to the executable,
5. common system font directories (Liberation Sans and DejaVu).

The bundled Liberation Sans family. `FontBook::instance().default_font()` never throws; if nothing is found it falls back to embedded built-in glyphs. To add a font at runtime:

```cpp
FontBook::instance().add("myfont", "/path/to/font.ttf");
Font f = FontBook::instance().get("myfont");
```

## License

Uranium is licensed under the [LGPL-2.1](LICENSE) license. However, [`stb_*`](third_party/stb) is licensed under the [MIT License or the Unlicense Public Domain License](third_party/stb/LICENSE), and [Liberation Sans](assets/fonts) under the [SIL Open Font License](assets/fonts/LICENSE).
