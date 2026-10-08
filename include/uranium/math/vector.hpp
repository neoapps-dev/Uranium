#pragma once
#include "uranium/math/functions.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <type_traits>
namespace uranium {
template <class T, std::size_t N>
struct Vector {
    static_assert(N > 0, "vector dimension must be positive");
    T e[N]{};
    constexpr Vector() = default;
    explicit constexpr Vector(T s) {
        for (std::size_t i = 0; i < N; ++i) e[i] = s;
    }

    template <class... U, class = std::enable_if_t<sizeof...(U) == N && sizeof...(U) != 1 && (std::is_convertible_v<U, T> && ...)>>
    constexpr Vector(U... u) : e{static_cast<T>(u)...} {}
    constexpr Vector(std::initializer_list<T> il) {
        std::size_t i = 0;
        for (T v : il) {
            if (i >= N) break;
            e[i++] = v;
        }
    }

    static constexpr std::size_t size() { return N; }
    constexpr T& operator[](std::size_t i) { return e[i]; }
    constexpr const T& operator[](std::size_t i) const { return e[i]; }
    constexpr T* data() { return e; }
    constexpr const T* data() const { return e; }
    constexpr T* begin() { return e; }
    constexpr T* end() { return e + N; }
    constexpr const T* begin() const { return e; }
    constexpr const T* end() const { return e + N; }
    constexpr Vector& operator+=(const Vector& o) {
        for (std::size_t i = 0; i < N; ++i) e[i] += o.e[i];
        return *this;
    }
    constexpr Vector& operator-=(const Vector& o) {
        for (std::size_t i = 0; i < N; ++i) e[i] -= o.e[i];
        return *this;
    }
    constexpr Vector& operator*=(T s) {
        for (std::size_t i = 0; i < N; ++i) e[i] *= s;
        return *this;
    }
    constexpr Vector& operator/=(T s) {
        for (std::size_t i = 0; i < N; ++i) e[i] /= s;
        return *this;
    }
    constexpr Vector& operator*=(const Vector& o) {
        for (std::size_t i = 0; i < N; ++i) e[i] *= o.e[i];
        return *this;
    }
    constexpr Vector& operator/=(const Vector& o) {
        for (std::size_t i = 0; i < N; ++i) e[i] /= o.e[i];
        return *this;
    }

    constexpr Vector operator-() const {
        Vector r;
        for (std::size_t i = 0; i < N; ++i) r.e[i] = -e[i];
        return r;
    }

    constexpr bool operator==(const Vector& o) const {
        for (std::size_t i = 0; i < N; ++i) if (!(e[i] == o.e[i])) return false;
        return true;
    }
    constexpr bool operator!=(const Vector& o) const { return !(*this == o); }
    constexpr T norm() const { return length(*this); }
    constexpr T squared_norm() const { return length_sq(*this); }
    constexpr Vector normalized() const { return normalize(*this); }
};

template <class T>
struct Vector<T, 2> {
    T x{}, y{};
    constexpr Vector() = default;
    constexpr Vector(T x_, T y_) : x(x_), y(y_) {}
    explicit constexpr Vector(T s) : x(s), y(s) {}
    template <class A, class B, class = std::enable_if_t<std::is_convertible_v<A, T> && std::is_convertible_v<B, T>>>
    constexpr Vector(A x_, B y_) : x(static_cast<T>(x_)), y(static_cast<T>(y_)) {}
    constexpr Vector(std::initializer_list<T> il) {
        auto it = il.begin();
        if (it != il.end()) x = *it++;
        if (it != il.end()) y = *it;
    }

    static constexpr std::size_t size() { return 2; }
    constexpr T& operator[](std::size_t i) { return i == 0 ? x : y; }
    constexpr const T& operator[](std::size_t i) const { return i == 0 ? x : y; }
    constexpr T* data() { return &x; }
    constexpr const T* data() const { return &x; }
    constexpr T* begin() { return &x; }
    constexpr T* end() { return &y + 1; }
    constexpr const T* begin() const { return &x; }
    constexpr const T* end() const { return &y + 1; }
    constexpr Vector& operator+=(const Vector& o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    constexpr Vector& operator-=(const Vector& o) {
        x -= o.x;
        y -= o.y;
        return *this;
    }
    constexpr Vector& operator*=(T s) {
        x *= s;
        y *= s;
        return *this;
    }
    constexpr Vector& operator/=(T s) {
        x /= s;
        y /= s;
        return *this;
    }
    constexpr Vector& operator*=(const Vector& o) {
        x *= o.x;
        y *= o.y;
        return *this;
    }
    constexpr Vector& operator/=(const Vector& o) {
        x /= o.x;
        y /= o.y;
        return *this;
    }

    constexpr Vector operator-() const { return Vector(-x, -y); }
    constexpr bool operator==(const Vector& o) const { return x == o.x && y == o.y; }
    constexpr bool operator!=(const Vector& o) const { return !(*this == o); }
    constexpr T norm() const { return length(*this); }
    constexpr T squared_norm() const { return length_sq(*this); }
    constexpr Vector normalized() const { return normalize(*this); }
};

template <class T>
struct Vector<T, 3> {
    T x{}, y{}, z{};
    constexpr Vector() = default;
    constexpr Vector(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}
    explicit constexpr Vector(T s) : x(s), y(s), z(s) {}
    template <class A, class B, class C, class = std::enable_if_t<std::is_convertible_v<A, T> && std::is_convertible_v<B, T> && std::is_convertible_v<C, T>>>
    constexpr Vector(A x_, B y_, C z_) : x(static_cast<T>(x_)), y(static_cast<T>(y_)), z(static_cast<T>(z_)) {}
    constexpr Vector(const Vector<T, 2>& v, T z_) : x(v.x), y(v.y), z(z_) {}
    constexpr Vector(std::initializer_list<T> il) {
        auto it = il.begin();
        if (it != il.end()) x = *it++;
        if (it != il.end()) y = *it++;
        if (it != il.end()) z = *it;
    }

    static constexpr std::size_t size() { return 3; }
    constexpr T& operator[](std::size_t i) { return i == 0 ? x : (i == 1 ? y : z); }
    constexpr const T& operator[](std::size_t i) const { return i == 0 ? x : (i == 1 ? y : z); }
    constexpr T* data() { return &x; }
    constexpr const T* data() const { return &x; }
    constexpr T* begin() { return &x; }
    constexpr T* end() { return &z + 1; }
    constexpr const T* begin() const { return &x; }
    constexpr const T* end() const { return &z + 1; }
    constexpr Vector& operator+=(const Vector& o) {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
    constexpr Vector& operator-=(const Vector& o) {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        return *this;
    }
    constexpr Vector& operator*=(T s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    constexpr Vector& operator/=(T s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }
    constexpr Vector& operator*=(const Vector& o) {
        x *= o.x;
        y *= o.y;
        z *= o.z;
        return *this;
    }
    constexpr Vector& operator/=(const Vector& o) {
        x /= o.x;
        y /= o.y;
        z /= o.z;
        return *this;
    }

    constexpr Vector operator-() const { return Vector(-x, -y, -z); }
    constexpr bool operator==(const Vector& o) const { return x == o.x && y == o.y && z == o.z; }
    constexpr bool operator!=(const Vector& o) const { return !(*this == o); }
    constexpr T norm() const { return length(*this); }
    constexpr T squared_norm() const { return length_sq(*this); }
    constexpr Vector normalized() const { return normalize(*this); }
};

template <class T>
struct Vector<T, 4> {
    T x{}, y{}, z{}, w{};
    constexpr Vector() = default;
    constexpr Vector(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}
    explicit constexpr Vector(T s) : x(s), y(s), z(s), w(s) {}
    template <class A, class B, class C, class D, class = std::enable_if_t<std::is_convertible_v<A, T> && std::is_convertible_v<B, T> && std::is_convertible_v<C, T> && std::is_convertible_v<D, T>>>
    constexpr Vector(A x_, B y_, C z_, D w_): x(static_cast<T>(x_)), y(static_cast<T>(y_)), z(static_cast<T>(z_)), w(static_cast<T>(w_)) {}
    constexpr Vector(const Vector<T, 3>& v, T w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
    constexpr Vector(const Vector<T, 2>& v, T z_, T w_) : x(v.x), y(v.y), z(z_), w(w_) {}
    constexpr Vector(std::initializer_list<T> il) {
        auto it = il.begin();
        if (it != il.end()) x = *it++;
        if (it != il.end()) y = *it++;
        if (it != il.end()) z = *it++;
        if (it != il.end()) w = *it;
    }

    static constexpr std::size_t size() { return 4; }
    constexpr T& operator[](std::size_t i) { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }
    constexpr const T& operator[](std::size_t i) const {
        return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w));
    }
    constexpr T* data() { return &x; }
    constexpr const T* data() const { return &x; }
    constexpr T* begin() { return &x; }
    constexpr T* end() { return &w + 1; }
    constexpr const T* begin() const { return &x; }
    constexpr const T* end() const { return &w + 1; }
    constexpr Vector& operator+=(const Vector& o) {
        x += o.x;
        y += o.y;
        z += o.z;
        w += o.w;
        return *this;
    }
    constexpr Vector& operator-=(const Vector& o) {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        w -= o.w;
        return *this;
    }
    constexpr Vector& operator*=(T s) {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }
    constexpr Vector& operator/=(T s) {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }
    constexpr Vector& operator*=(const Vector& o) {
        x *= o.x;
        y *= o.y;
        z *= o.z;
        w *= o.w;
        return *this;
    }
    constexpr Vector& operator/=(const Vector& o) {
        x /= o.x;
        y /= o.y;
        z /= o.z;
        w /= o.w;
        return *this;
    }

    constexpr Vector operator-() const { return Vector(-x, -y, -z, -w); }
    constexpr bool operator==(const Vector& o) const {
        return x == o.x && y == o.y && z == o.z && w == o.w;
    }
    constexpr bool operator!=(const Vector& o) const { return !(*this == o); }
    constexpr T norm() const { return length(*this); }
    constexpr T squared_norm() const { return length_sq(*this); }
    constexpr Vector normalized() const { return normalize(*this); }
};

template <class T, std::size_t N>
constexpr Vector<T, N> operator+(Vector<T, N> a, const Vector<T, N>& b) {
    return a += b;
}
template <class T, std::size_t N>
constexpr Vector<T, N> operator-(Vector<T, N> a, const Vector<T, N>& b) {
    return a -= b;
}
template <class T, std::size_t N>
constexpr Vector<T, N> operator*(Vector<T, N> a, T s) {
    return a *= s;
}
template <class T, std::size_t N>
constexpr Vector<T, N> operator*(T s, Vector<T, N> a) {
    return a *= s;
}
template <class T, std::size_t N>
constexpr Vector<T, N> operator/(Vector<T, N> a, T s) {
    return a /= s;
}
template <class T, std::size_t N>
constexpr Vector<T, N> operator*(Vector<T, N> a, const Vector<T, N>& b) {
    return a *= b;
}
template <class T, std::size_t N>
constexpr Vector<T, N> operator/(Vector<T, N> a, const Vector<T, N>& b) {
    return a /= b;
}

using Vec2 = Vector<float, 2>;
using Vec3 = Vector<float, 3>;
using Vec4 = Vector<float, 4>;
using DVec2 = Vector<double, 2>;
using DVec3 = Vector<double, 3>;
using DVec4 = Vector<double, 4>;
using IVec2 = Vector<int, 2>;
using IVec3 = Vector<int, 3>;
template <class T, std::size_t N>
constexpr T dot(const Vector<T, N>& a, const Vector<T, N>& b) {
    T s = T(0);
    for (std::size_t i = 0; i < N; ++i) s += a[i] * b[i];
    return s;
}

template <class T>
constexpr T cross(const Vector<T, 2>& a, const Vector<T, 2>& b) {
    return a.x * b.y - a.y * b.x;
}

template <class T>
constexpr Vector<T, 3> cross(const Vector<T, 3>& a, const Vector<T, 3>& b) {
    return Vector<T, 3>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template <class T, std::size_t N>
constexpr T length_sq(const Vector<T, N>& v) {
    return dot(v, v);
}

template <class T, std::size_t N>
constexpr T length(const Vector<T, N>& v) {
    return std::sqrt(dot(v, v));
}

template <class T, std::size_t N>
constexpr T distance(const Vector<T, N>& a, const Vector<T, N>& b) {
    return length(a - b);
}

template <class T, std::size_t N>
constexpr T distance_sq(const Vector<T, N>& a, const Vector<T, N>& b) {
    return length_sq(a - b);
}

template <class T, std::size_t N>
constexpr Vector<T, N> normalize(const Vector<T, N>& v) {
    T len = length(v);
    if (len <= T(0)) return Vector<T, N>();
    return v / len;
}

template <class T, std::size_t N>
constexpr Vector<T, N> normalize_or(const Vector<T, N>& v, const Vector<T, N>& fallback) {
    T len = length(v);
    return len <= T(0) ? fallback : v / len;
}

template <class T, std::size_t N>
constexpr Vector<T, N> abs(const Vector<T, N>& v) {
    Vector<T, N> r;
    for (std::size_t i = 0; i < N; ++i) r[i] = std::fabs(v[i]);
    return r;
}

template <class T, std::size_t N>
constexpr Vector<T, N> floor(const Vector<T, N>& v) {
    Vector<T, N> r;
    for (std::size_t i = 0; i < N; ++i) r[i] = std::floor(v[i]);
    return r;
}

template <class T, std::size_t N>
constexpr Vector<T, N> ceil(const Vector<T, N>& v) {
    Vector<T, N> r;
    for (std::size_t i = 0; i < N; ++i) r[i] = std::ceil(v[i]);
    return r;
}

template <class T, std::size_t N>
constexpr Vector<T, N> round(const Vector<T, N>& v) {
    Vector<T, N> r;
    for (std::size_t i = 0; i < N; ++i) r[i] = std::round(v[i]);
    return r;
}

template <class T, std::size_t N>
constexpr Vector<T, N> min(const Vector<T, N>& a, const Vector<T, N>& b) {
    Vector<T, N> r;
    for (std::size_t i = 0; i < N; ++i) r[i] = a[i] < b[i] ? a[i] : b[i];
    return r;
}

template <class T, std::size_t N>
constexpr Vector<T, N> max(const Vector<T, N>& a, const Vector<T, N>& b) {
    Vector<T, N> r;
    for (std::size_t i = 0; i < N; ++i) r[i] = a[i] > b[i] ? a[i] : b[i];
    return r;
}

template <class T, std::size_t N>
constexpr Vector<T, N> clamp(const Vector<T, N>& v, const Vector<T, N>& lo, const Vector<T, N>& hi) {
    Vector<T, N> r;
    for (std::size_t i = 0; i < N; ++i) r[i] = v[i] < lo[i] ? lo[i] : (v[i] > hi[i] ? hi[i] : v[i]);
    return r;
}

template <class T, std::size_t N>
constexpr Vector<T, N> lerp(const Vector<T, N>& a, const Vector<T, N>& b, T t) {
    Vector<T, N> r;
    for (std::size_t i = 0; i < N; ++i) r[i] = a[i] + (b[i] - a[i]) * t;
    return r;
}

template <class T, std::size_t N>
constexpr Vector<T, N> reflect(const Vector<T, N>& v, const Vector<T, N>& n) {
    return v - n * (T(2) * dot(v, n));
}

template <class T, std::size_t N>
constexpr Vector<T, N> project(const Vector<T, N>& v, const Vector<T, N>& onto) {
    T len2 = length_sq(onto);
    if (len2 <= T(0)) return Vector<T, N>();
    return onto * (dot(v, onto) / len2);
}

template <class T, std::size_t N>
constexpr Vector<T, N> reject(const Vector<T, N>& v, const Vector<T, N>& onto) {
    return v - project(v, onto);
}

template <class T, std::size_t N>
constexpr T angle_between(const Vector<T, N>& a, const Vector<T, N>& b) {
    T d = dot(a, b);
    T la = length(a) * length(b);
    if (la <= T(0)) return T(0);
    return std::acos(clamp(d / la, T(-1), T(1)));
}

template <class T, std::size_t N>
constexpr Vector<T, N> midpoint(const Vector<T, N>& a, const Vector<T, N>& b) {
    return (a + b) / T(2);
}

template <class T, std::size_t N>
constexpr T min_component(const Vector<T, N>& v) {
    T m = v[0];
    for (std::size_t i = 1; i < N; ++i) m = std::min(m, v[i]);
    return m;
}

template <class T, std::size_t N>
constexpr T max_component(const Vector<T, N>& v) {
    T m = v[0];
    for (std::size_t i = 1; i < N; ++i) m = std::max(m, v[i]);
    return m;
}

template <class T, std::size_t N>
constexpr bool all(const Vector<T, N>& v) {
    for (std::size_t i = 0; i < N; ++i) if (!v[i]) return false;
    return true;
}

template <class T, std::size_t N>
constexpr bool any(const Vector<T, N>& v) {
    for (std::size_t i = 0; i < N; ++i) if (v[i]) return true;
    return false;
}

template <class T, std::size_t N>
constexpr bool is_finite(const Vector<T, N>& v) {
    for (std::size_t i = 0; i < N; ++i) if (!std::isfinite(v[i])) return false;
    return true;
}

template <class T, std::size_t N>
constexpr bool almost_equal(const Vector<T, N>& a, const Vector<T, N>& b, T eps = T(1e-6)) {
    for (std::size_t i = 0; i < N; ++i) if (!almost_equal(a[i], b[i], eps)) return false;
    return true;
}

template <class T, std::size_t N>
constexpr Vector<T, N> perpendicular(const Vector<T, N>& v) {
    static_assert(N == 2, "perpendicular requires 2d vector");
    return Vector<T, N>(-v[1], v[0]);
}

template <class T, std::size_t N>
constexpr Vector<T, N> move_towards(const Vector<T, N>& current, const Vector<T, N>& target, T max_delta) {
    Vector<T, N> delta = target - current;
    T len = length(delta);
    if (len <= max_delta || len <= T(0)) return target;
    return current + delta * (max_delta / len);
}
}
