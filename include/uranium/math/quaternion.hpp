#pragma once
#include "uranium/math/matrix.hpp"
#include "uranium/math/vector.hpp"
#include <cmath>
#include <cstddef>
namespace uranium {
template <class T>
struct Quaternion {
    T w{T(1)}, x{}, y{}, z{};
    constexpr Quaternion() = default;
    constexpr Quaternion(T w_, T x_, T y_, T z_) : w(w_), x(x_), y(y_), z(z_) {}
    static constexpr Quaternion identity() { return Quaternion(T(1), T(0), T(0), T(0)); }
    static Quaternion from_axis_angle(const Vector<T, 3>& axis, T angle) {
        Vector<T, 3> n = normalize(axis);
        T h = angle * T(0.5);
        T s = std::sin(h);
        return Quaternion(std::cos(h), n.x * s, n.y * s, n.z * s);
    }

    static Quaternion from_euler(T roll, T pitch, T yaw) {
        T cr = std::cos(roll * T(0.5)), sr = std::sin(roll * T(0.5));
        T cp = std::cos(pitch * T(0.5)), sp = std::sin(pitch * T(0.5));
        T cy = std::cos(yaw * T(0.5)), sy = std::sin(yaw * T(0.5));
        return Quaternion(cr * cp * cy + sr * sp * sy, sr * cp * cy - cr * sp * sy, cr * sp * cy + sr * cp * sy, cr * cp * sy - sr * sp * cy);
    }

    static Quaternion from_matrix(const Matrix<T, 3, 3>& m) {
        T tr = m.m[0][0] + m.m[1][1] + m.m[2][2];
        Quaternion q;
        if (tr > T(0)) {
            T s = std::sqrt(tr + T(1)) * T(2);
            q.w = T(0.25) * s;
            q.x = (m.m[2][1] - m.m[1][2]) / s;
            q.y = (m.m[0][2] - m.m[2][0]) / s;
            q.z = (m.m[1][0] - m.m[0][1]) / s;
        } else if (m.m[0][0] > m.m[1][1] && m.m[0][0] > m.m[2][2]) {
            T s = std::sqrt(T(1) + m.m[0][0] - m.m[1][1] - m.m[2][2]) * T(2);
            q.w = (m.m[2][1] - m.m[1][2]) / s;
            q.x = T(0.25) * s;
            q.y = (m.m[0][1] + m.m[1][0]) / s;
            q.z = (m.m[0][2] + m.m[2][0]) / s;
        } else if (m.m[1][1] > m.m[2][2]) {
            T s = std::sqrt(T(1) + m.m[1][1] - m.m[0][0] - m.m[2][2]) * T(2);
            q.w = (m.m[0][2] - m.m[2][0]) / s;
            q.x = (m.m[0][1] + m.m[1][0]) / s;
            q.y = T(0.25) * s;
            q.z = (m.m[1][2] + m.m[2][1]) / s;
        } else {
            T s = std::sqrt(T(1) + m.m[2][2] - m.m[0][0] - m.m[1][1]) * T(2);
            q.w = (m.m[1][0] - m.m[0][1]) / s;
            q.x = (m.m[0][2] + m.m[2][0]) / s;
            q.y = (m.m[1][2] + m.m[2][1]) / s;
            q.z = T(0.25) * s;
        }
        return q.normalized();
    }

    static Quaternion from_two_vectors(const Vector<T, 3>& a, const Vector<T, 3>& b) {
        Vector<T, 3> na = normalize(a);
        Vector<T, 3> nb = normalize(b);
        T d = dot(na, nb);
        if (d >= T(1) - T(1e-8)) return identity();
        if (d <= T(-1) + T(1e-8)) {
            Vector<T, 3> axis = cross(Vector<T, 3>(T(1), T(0), T(0)), na);
            if (length_sq(axis) < T(1e-8)) axis = cross(Vector<T, 3>(T(0), T(1), T(0)), na);
            return from_axis_angle(axis, T(pi));
        }
        Vector<T, 3> c = cross(na, nb);
        T s = std::sqrt((T(1) + d) * T(2));
        return Quaternion(T(0.5) * s, c.x / s, c.y / s, c.z / s).normalized();
    }

    constexpr Quaternion operator*(const Quaternion& o) const {
        return Quaternion(w * o.w - x * o.x - y * o.y - z * o.z,
                          w * o.x + x * o.w + y * o.z - z * o.y,
                          w * o.y - x * o.z + y * o.w + z * o.x,
                          w * o.z + x * o.y - y * o.x + z * o.w);
    }

    constexpr Quaternion& operator*=(const Quaternion& o) { return *this = *this * o; }
    constexpr Quaternion operator+(const Quaternion& o) const {
        return Quaternion(w + o.w, x + o.x, y + o.y, z + o.z);
    }
    constexpr Quaternion operator-(const Quaternion& o) const {
        return Quaternion(w - o.w, x - o.x, y - o.y, z - o.z);
    }
    constexpr Quaternion operator*(T s) const { return Quaternion(w * s, x * s, y * s, z * s); }
    constexpr Quaternion operator-() const { return Quaternion(-w, -x, -y, -z); }
    constexpr T length_sq() const { return w * w + x * x + y * y + z * z; }
    constexpr T length() const { return std::sqrt(length_sq()); }
    constexpr Quaternion normalized() const {
        T len = length();
        if (len <= T(0)) return identity();
        T inv = T(1) / len;
        return Quaternion(w * inv, x * inv, y * inv, z * inv);
    }

    constexpr Quaternion conjugate() const { return Quaternion(w, -x, -y, -z); }
    constexpr Quaternion inverse() const {
        T len2 = length_sq();
        if (len2 <= T(0)) return identity();
        T inv = T(1) / len2;
        return Quaternion(w * inv, -x * inv, -y * inv, -z * inv);
    }

    constexpr T dot(const Quaternion& o) const { return w * o.w + x * o.x + y * o.y + z * o.z; }
    constexpr Vector<T, 3> rotate(const Vector<T, 3>& v) const {
        Vector<T, 3> qv(x, y, z);
        Vector<T, 3> t = cross(qv, v) * T(2);
        return v + t * w + cross(qv, t);
    }

    constexpr Vector<T, 3> euler_angles() const {
        Quaternion q = normalized();
        T sinr = T(2) * (q.w * q.x + q.y * q.z);
        T cosr = T(1) - T(2) * (q.x * q.x + q.y * q.y);
        T roll = std::atan2(sinr, cosr);
        T sinp = T(2) * (q.w * q.y - q.z * q.x);
        T pitch = sinp >= T(1) ? T(pi) / T(2) : (sinp <= T(-1) ? T(-pi) / T(2) : std::asin(sinp));
        T siny = T(2) * (q.w * q.z + q.x * q.y);
        T cosy = T(1) - T(2) * (q.y * q.y + q.z * q.z);
        T yaw = std::atan2(siny, cosy);
        return Vector<T, 3>(roll, pitch, yaw);
    }

    constexpr T angle() const {
        Quaternion q = normalized();
        T c = clamp(q.w, T(-1), T(1));
        return T(2) * std::acos(c);
    }

    constexpr Vector<T, 3> axis() const {
        Quaternion q = normalized();
        T s = std::sqrt(T(1) - q.w * q.w);
        if (s < T(1e-6)) return Vector<T, 3>(T(1), T(0), T(0));
        return Vector<T, 3>(q.x / s, q.y / s, q.z / s);
    }

    constexpr Matrix<T, 3, 3> mat3() const {
        Quaternion q = normalized();
        T xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
        T xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
        T wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
        return Matrix<T, 3, 3>{{T(1) - T(2) * (yy + zz), T(2) * (xy - wz), T(2) * (xz + wy)},
                               {T(2) * (xy + wz), T(1) - T(2) * (xx + zz), T(2) * (yz - wx)},
                               {T(2) * (xz - wy), T(2) * (yz + wx), T(1) - T(2) * (xx + yy)}};
    }

    constexpr Matrix<T, 4, 4> mat4() const { return Matrix<T, 4, 4>::embed3(mat3()); }
    constexpr Matrix<T, 3, 3> to_matrix() const { return mat3(); }
    constexpr Matrix<T, 4, 4> to_mat4() const { return mat4(); }
    static Quaternion slerp(const Quaternion& a, const Quaternion& b, T t) {
        Quaternion qa = a.normalized();
        Quaternion qb = b.normalized();
        T d = qa.dot(qb);
        if (d < T(0)) {
            qb = -qb;
            d = -d;
        }
        if (d > T(0.9995)) {
            return Quaternion(qa.w + (qb.w - qa.w) * t, qa.x + (qb.x - qa.x) * t, qa.y + (qb.y - qa.y) * t, qa.z + (qb.z - qa.z) * t).normalized();
        }
        T theta = std::acos(clamp(d, T(-1), T(1)));
        T sin_theta = std::sin(theta);
        T wa = std::sin((T(1) - t) * theta) / sin_theta;
        T wb = std::sin(t * theta) / sin_theta;
        return (qa * wa + qb * wb).normalized();
    }

    static Quaternion nlerp(const Quaternion& a, const Quaternion& b, T t) {
        Quaternion qa = a.normalized();
        Quaternion qb = b.normalized();
        if (qa.dot(qb) < T(0)) qb = -qb;
        return (qa * (T(1) - t) + qb * t).normalized();
    }

    constexpr bool operator==(const Quaternion& o) const {
        return w == o.w && x == o.x && y == o.y && z == o.z;
    }
    constexpr bool operator!=(const Quaternion& o) const { return !(*this == o); }
};

using Quat = Quaternion<float>;
using DQuat = Quaternion<double>;
template <class T>
constexpr T dot(const Quaternion<T>& a, const Quaternion<T>& b) {
    return a.dot(b);
}

template <class T>
constexpr Vector<T, 3> rotate(const Quaternion<T>& q, const Vector<T, 3>& v) {
    return q.rotate(v);
}

template <class T>
Quaternion<T> slerp(const Quaternion<T>& a, const Quaternion<T>& b, T t) {
    return Quaternion<T>::slerp(a, b, t);
}

template <class T>
Quaternion<T> nlerp(const Quaternion<T>& a, const Quaternion<T>& b, T t) {
    return Quaternion<T>::nlerp(a, b, t);
}

template <class T>
Matrix<T, 3, 3> to_matrix(const Quaternion<T>& q) {
    return q.mat3();
}

template <class T>
Matrix<T, 4, 4> to_mat4(const Quaternion<T>& q) {
    return q.mat4();
}
}
