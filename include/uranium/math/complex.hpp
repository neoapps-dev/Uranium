#pragma once
#include "uranium/math/constants.hpp"
#include <cmath>
#include <cstddef>
namespace uranium {
template <class T>
struct Complex {
    T re{}, im{};
    constexpr Complex() = default;
    constexpr Complex(T re_, T im_ = T(0)) : re(re_), im(im_) {}
    static constexpr Complex polar(T magnitude, T angle) {
        return Complex(magnitude * std::cos(angle), magnitude * std::sin(angle));
    }

    constexpr Complex operator+(const Complex& o) const { return Complex(re + o.re, im + o.im); }
    constexpr Complex operator-(const Complex& o) const { return Complex(re - o.re, im - o.im); }
    constexpr Complex operator-() const { return Complex(-re, -im); }
    constexpr Complex operator*(const Complex& o) const {
        return Complex(re * o.re - im * o.im, re * o.im + im * o.re);
    }
    constexpr Complex operator*(T s) const { return Complex(re * s, im * s); }
    constexpr Complex operator/(const Complex& o) const {
        T d = o.re * o.re + o.im * o.im;
        return Complex((re * o.re + im * o.im) / d, (im * o.re - re * o.im) / d);
    }
    constexpr Complex operator/(T s) const { return Complex(re / s, im / s); }
    constexpr Complex& operator+=(const Complex& o) { return *this = *this + o; }
    constexpr Complex& operator-=(const Complex& o) { return *this = *this - o; }
    constexpr Complex& operator*=(const Complex& o) { return *this = *this * o; }
    constexpr Complex& operator/=(const Complex& o) { return *this = *this / o; }
    constexpr Complex& operator*=(T s) { return *this = *this * s; }
    constexpr Complex& operator/=(T s) { return *this = *this / s; }
    constexpr bool operator==(const Complex& o) const { return re == o.re && im == o.im; }
    constexpr bool operator!=(const Complex& o) const { return !(*this == o); }
    constexpr T norm() const { return re * re + im * im; }
    constexpr T abs() const { return std::sqrt(norm()); }
    constexpr T arg() const { return std::atan2(im, re); }
    constexpr Complex conj() const { return Complex(re, -im); }
    constexpr Complex inverse() const {
        T d = norm();
        return Complex(re / d, -im / d);
    }

    constexpr T* data() { return &re; }
    constexpr const T* data() const { return &re; }
};

template <class T>
constexpr Complex<T> operator*(T s, const Complex<T>& c) {
    return Complex<T>(c.re * s, c.im * s);
}

template <class T>
constexpr T real(const Complex<T>& c) {
    return c.re;
}

template <class T>
constexpr T imag(const Complex<T>& c) {
    return c.im;
}

template <class T>
constexpr T norm(const Complex<T>& c) {
    return c.norm();
}

template <class T>
constexpr T abs(const Complex<T>& c) {
    return c.abs();
}

template <class T>
constexpr T arg(const Complex<T>& c) {
    return c.arg();
}

template <class T>
constexpr Complex<T> conj(const Complex<T>& c) {
    return c.conj();
}

template <class T>
Complex<T> exp(const Complex<T>& c) {
    T e = std::exp(c.re);
    return Complex<T>(e * std::cos(c.im), e * std::sin(c.im));
}

template <class T>
Complex<T> log(const Complex<T>& c) {
    return Complex<T>(std::log(c.abs()), c.arg());
}

template <class T>
Complex<T> log10(const Complex<T>& c) {
    return log(c) / std::log(T(10));
}

template <class T>
Complex<T> sqrt(const Complex<T>& c) {
    T r = c.abs();
    if (r == T(0)) return Complex<T>(T(0), T(0));
    if (c.re >= T(0)) {
        T t = std::sqrt((r + c.re) * T(0.5));
        return Complex<T>(t, c.im / (T(2) * t));
    }
    T t = std::sqrt((r - c.re) * T(0.5));
    T im = c.im >= T(0) ? t : -t;
    return Complex<T>(c.im / (T(2) * t), im);
}

template <class T>
Complex<T> pow(const Complex<T>& base, const Complex<T>& ex) {
    if (base.re == T(0) && base.im == T(0)) return Complex<T>(T(0), T(0));
    return exp(log(base) * ex);
}

template <class T>
Complex<T> pow(const Complex<T>& base, T ex) {
    if (base.re == T(0) && base.im == T(0)) return Complex<T>(T(0), T(0));
    T r = std::pow(base.abs(), ex);
    T a = base.arg() * ex;
    return Complex<T>::polar(r, a);
}

template <class T>
Complex<T> sin(const Complex<T>& c) {
    return Complex<T>(std::sin(c.re) * std::cosh(c.im), std::cos(c.re) * std::sinh(c.im));
}

template <class T>
Complex<T> cos(const Complex<T>& c) {
    return Complex<T>(std::cos(c.re) * std::cosh(c.im), -std::sin(c.re) * std::sinh(c.im));
}

template <class T>
Complex<T> tan(const Complex<T>& c) {
    return sin(c) / cos(c);
}

template <class T>
Complex<T> sinh(const Complex<T>& c) {
    return Complex<T>(std::sinh(c.re) * std::cos(c.im), std::cosh(c.re) * std::sin(c.im));
}

template <class T>
Complex<T> cosh(const Complex<T>& c) {
    return Complex<T>(std::cosh(c.re) * std::cos(c.im), std::sinh(c.re) * std::sin(c.im));
}

using Cf = Complex<float>;
using Cd = Complex<double>;
}
