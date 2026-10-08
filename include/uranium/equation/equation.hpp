#pragma once
#include "uranium/core/config.hpp"
#include "uranium/render/canvas.hpp"
#include "uranium/render/color.hpp"
#include <memory>
#include <string>
namespace uranium {
namespace equation {
struct Layout {
    float width = 0.0f;
    float ascent = 0.0f;
    float descent = 0.0f;
    URANIUM_NODISCARD float height() const { return ascent + descent; }
};

class URANIUM_API Equation {
public:
    Equation();
    explicit Equation(const std::string& latex);
    Equation(const Equation& other);
    Equation(Equation&& other) noexcept;
    Equation& operator=(const Equation& other);
    Equation& operator=(Equation&& other) noexcept;
    ~Equation();
    URANIUM_NODISCARD bool valid() const;
    URANIUM_NODISCARD const std::string& error() const;
    URANIUM_NODISCARD Layout measure(float px) const;
    void draw(render::Canvas& canvas, render::Vec2 baseline_left, float px, render::Color color = render::colors::black) const;
    void draw_aligned(render::Canvas& canvas, render::Vec2 anchor, float px, render::Color color, render::TextAlign align) const;
private:
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

URANIUM_NODISCARD Layout measure(const std::string& latex, float px);
void draw(render::Canvas& canvas, render::Vec2 baseline_left, const std::string& latex, float px, render::Color color = render::colors::black);
void draw_aligned(render::Canvas& canvas, render::Vec2 anchor, const std::string& latex, float px, render::Color color, render::TextAlign align);
}
}
