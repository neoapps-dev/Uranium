#pragma once
#include "uranium/math/constants.hpp"
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
namespace uranium {
template <class T>
constexpr T clamp(T v, T lo, T hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

template <class T>
constexpr T saturate(T v) {
    return clamp(v, T(0), T(1));
}

template <class T>
constexpr T lerp(T a, T b, T t) {
    return a + (b - a) * t;
}

template <class T>
constexpr T inv_lerp(T a, T b, T v) {
    return a == b ? T(0) : (v - a) / (b - a);
}

template <class T>
constexpr T remap(T v, T in_lo, T in_hi, T out_lo, T out_hi) {
    return lerp(out_lo, out_hi, inv_lerp(in_lo, in_hi, v));
}

template <class T>
constexpr T smoothstep(T edge0, T edge1, T x) {
    T t = saturate(inv_lerp(edge0, edge1, x));
    return t * t * (T(3) - T(2) * t);
}

template <class T>
constexpr T smootherstep(T edge0, T edge1, T x) {
    T t = saturate(inv_lerp(edge0, edge1, x));
    return t * t * t * (t * (t * T(6) - T(15)) + T(10));
}

template <class T>
constexpr T step(T edge, T x) {
    return x < edge ? T(0) : T(1);
}

template <class T>
constexpr int sign(T v) {
    return v > T(0) ? 1 : (v < T(0) ? -1 : 0);
}

template <class T>
constexpr T square(T v) {
    return v * v;
}

template <class T>
constexpr T deg2rad(T deg) {
    return deg * (T(pi) / T(180));
}

template <class T>
constexpr T rad2deg(T rad) {
    return rad * (T(180) / T(pi));
}

template <class T>
constexpr T wrap_pi(T a) {
    a = std::fmod(a + T(pi), T(tau));
    if (a < T(0)) a += T(tau);
    return a - T(pi);
}

template <class T>
constexpr T fract(T v) {
    return v - std::floor(v);
}

template <class T>
constexpr T min3(T a, T b, T c) {
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

template <class T>
constexpr T max3(T a, T b, T c) {
    return a > b ? (a > c ? a : c) : (b > c ? b : c);
}

template <class T>
constexpr bool is_power_of_two(T v) {
    return v > 0 && (v & (v - 1)) == 0;
}

template <class T>
constexpr T next_power_of_two(T v) {
    T p = 1;
    while (p < v) p <<= 1;
    return p;
}

template <class T>
constexpr bool almost_equal(T a, T b, T eps = T(1e-9)) {
    T d = a - b;
    if (d < T(0)) d = -d;
    T m = std::fabs(a) > std::fabs(b) ? std::fabs(a) : std::fabs(b);
    return d <= eps * (m > T(1) ? m : T(1));
}

template <class T>
constexpr bool is_finite(T v) {
    return v == v && v - v == T(0);
}

template <class T>
constexpr T pi_v = static_cast<T>(pi);
}
