#pragma once
#include "uranium/core/config.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
namespace uranium {
namespace render {
struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
    constexpr Color() = default;
    constexpr Color(std::uint8_t r_, std::uint8_t g_, std::uint8_t b_, std::uint8_t a_ = 255): r(r_), g(g_), b(b_), a(a_) {}
    static constexpr Color rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) {
        return Color(r, g, b, a);
    }

    static constexpr Color gray(std::uint8_t v, std::uint8_t a = 255) { return Color(v, v, v, a); }
    static Color hsv(float h, float s, float v, std::uint8_t alpha = 255) {
        h = std::fmod(h, 360.0f);
        if (h < 0.0f) h += 360.0f;
        float c = v * s;
        float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;
        float rf = 0.0f, gf = 0.0f, bf = 0.0f;
        if (h < 60.0f) {
            rf = c; gf = x;
        } else if (h < 120.0f) {
            rf = x; gf = c;
        } else if (h < 180.0f) {
            gf = c; bf = x;
        } else if (h < 240.0f) {
            gf = x; bf = c;
        } else if (h < 300.0f) {
            rf = x; bf = c;
        } else {
            rf = c; bf = x;
        }
        return Color(static_cast<std::uint8_t>((rf + m) * 255.0f + 0.5f), static_cast<std::uint8_t>((gf + m) * 255.0f + 0.5f), static_cast<std::uint8_t>((bf + m) * 255.0f + 0.5f), alpha);
    }

    static Color from_hex(std::uint32_t rgb, std::uint8_t alpha = 255) {
        return Color(static_cast<std::uint8_t>((rgb >> 16) & 0xFF), static_cast<std::uint8_t>((rgb >> 8) & 0xFF), static_cast<std::uint8_t>(rgb & 0xFF), alpha);
    }

    static Color from_hex(const std::string& text, std::uint8_t alpha = 255) {
        std::string s = text;
        if (!s.empty() && s[0] == '#') s = s.substr(1);
        if (s.size() == 3) s = std::string(2, s[0]) + std::string(2, s[1]) + std::string(2, s[2]);
        std::uint32_t value = 0;
        for (char c : s) {
            value <<= 4;
            if (c >= '0' && c <= '9') value |= static_cast<std::uint32_t>(c - '0');
            else if (c >= 'a' && c <= 'f') value |= static_cast<std::uint32_t>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') value |= static_cast<std::uint32_t>(c - 'A' + 10);
        }
        return from_hex(value, alpha);
    }

    URANIUM_NODISCARD constexpr float fr() const { return static_cast<float>(r) / 255.0f; }
    URANIUM_NODISCARD constexpr float fg() const { return static_cast<float>(g) / 255.0f; }
    URANIUM_NODISCARD constexpr float fb() const { return static_cast<float>(b) / 255.0f; }
    URANIUM_NODISCARD constexpr float fa() const { return static_cast<float>(a) / 255.0f; }
    URANIUM_NODISCARD constexpr Color with_alpha(std::uint8_t alpha) const { return Color(r, g, b, alpha); }
    URANIUM_NODISCARD Color mixed(Color other, float t) const {
        t = std::max(0.0f, std::min(1.0f, t));
        return Color(static_cast<std::uint8_t>(r + (other.r - static_cast<int>(r)) * t + 0.5f), static_cast<std::uint8_t>(g + (other.g - static_cast<int>(g)) * t + 0.5f),
                     static_cast<std::uint8_t>(b + (other.b - static_cast<int>(b)) * t + 0.5f), static_cast<std::uint8_t>(a + (other.a - static_cast<int>(a)) * t + 0.5f));
    }

    URANIUM_NODISCARD std::string hex() const {
        const char* digits = "0123456789abcdef";
        std::string out;
        auto push = [&out, &digits](std::uint8_t v) {
            out.push_back(digits[v >> 4]);
            out.push_back(digits[v & 0xF]);
        };
        push(r);
        push(g);
        push(b);
        return out;
    }

    URANIUM_NODISCARD std::string css() const {
        if (a == 255) return "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
        return "rgba(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + "," + std::to_string(static_cast<int>(a)) + ")";
    }

    constexpr bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
    constexpr bool operator!=(const Color& o) const { return !(*this == o); }
};

namespace colors {
inline constexpr Color transparent{0, 0, 0, 0};
inline constexpr Color black{0, 0, 0};
inline constexpr Color white{255, 255, 255};
inline constexpr Color red{220, 50, 47};
inline constexpr Color green{0, 153, 0};
inline constexpr Color blue{38, 139, 210};
inline constexpr Color yellow{181, 137, 0};
inline constexpr Color orange{203, 75, 22};
inline constexpr Color purple{108, 113, 196};
inline constexpr Color pink{211, 54, 130};
inline constexpr Color cyan{42, 161, 152};
inline constexpr Color magenta{211, 54, 130};
inline constexpr Color gray{147, 161, 161};
inline constexpr Color light_gray{238, 232, 213};
inline constexpr Color dark_gray{88, 110, 117};
inline constexpr Color sky_blue{131, 148, 150};
inline Color palette(int index) {
    static const Color base[] = {
        {38, 139, 210}, {220, 50, 47}, {133, 153, 0}, {181, 137, 0},
        {203, 75, 22},  {211, 54, 130}, {108, 113, 196}, {42, 161, 152},
        {147, 161, 161}, {220, 50, 47},
    };
    int n = static_cast<int>(sizeof(base) / sizeof(base[0]));
    int i = ((index % n) + n) % n;
    return base[i];
}
}
}
}
