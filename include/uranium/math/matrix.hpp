#pragma once
#include "uranium/math/vector.hpp"
#include <cmath>
#include <cstddef>
#include <initializer_list>
namespace uranium {
template <class T, std::size_t R, std::size_t C>
struct Matrix {
    static_assert(R > 0 && C > 0, "matrix dimensions must be positive");
    T m[R][C]{};
    constexpr Matrix() = default;
    constexpr Matrix(std::initializer_list<std::initializer_list<T>> il) {
        std::size_t r = 0;
        for (const auto& row_il : il) {
            if (r >= R) break;
            std::size_t c = 0;
            for (const auto& v : row_il) {
                if (c >= C) break;
                m[r][c++] = v;
            }
            ++r;
        }
    }

    static constexpr Matrix zero() { return Matrix(); }
    static constexpr Matrix identity() {
        Matrix r;
        for (std::size_t i = 0; i < R && i < C; ++i) r.m[i][i] = T(1);
        return r;
    }

    static constexpr std::size_t rows() { return R; }
    static constexpr std::size_t cols() { return C; }
    constexpr T& operator()(std::size_t r, std::size_t c) { return m[r][c]; }
    constexpr const T& operator()(std::size_t r, std::size_t c) const { return m[r][c]; }
    constexpr T* operator[](std::size_t r) { return m[r]; }
    constexpr const T* operator[](std::size_t r) const { return m[r]; }
    constexpr T* data() { return &m[0][0]; }
    constexpr const T* data() const { return &m[0][0]; }
    constexpr Vector<T, C> row(std::size_t r) const {
        Vector<T, C> v;
        for (std::size_t c = 0; c < C; ++c) v[c] = m[r][c];
        return v;
    }

    constexpr Vector<T, R> col(std::size_t c) const {
        Vector<T, R> v;
        for (std::size_t r = 0; r < R; ++r) v[r] = m[r][c];
        return v;
    }

    constexpr void set_row(std::size_t r, const Vector<T, C>& v) {
        for (std::size_t c = 0; c < C; ++c) m[r][c] = v[c];
    }

    constexpr void set_col(std::size_t c, const Vector<T, R>& v) {
        for (std::size_t r = 0; r < R; ++r) m[r][c] = v[r];
    }

    constexpr Matrix<T, C, R> transposed() const {
        Matrix<T, C, R> r;
        for (std::size_t i = 0; i < R; ++i)
            for (std::size_t j = 0; j < C; ++j) r.m[j][i] = m[i][j];
        return r;
    }

    constexpr T trace() const {
        T s = T(0);
        for (std::size_t i = 0; i < R && i < C; ++i) s += m[i][i];
        return s;
    }

    constexpr Matrix& operator+=(const Matrix& o) {
        for (std::size_t i = 0; i < R; ++i) for (std::size_t j = 0; j < C; ++j) m[i][j] += o.m[i][j];
        return *this;
    }

    constexpr Matrix& operator-=(const Matrix& o) {
        for (std::size_t i = 0; i < R; ++i) for (std::size_t j = 0; j < C; ++j) m[i][j] -= o.m[i][j];
        return *this;
    }

    constexpr Matrix& operator*=(T s) {
        for (std::size_t i = 0; i < R; ++i) for (std::size_t j = 0; j < C; ++j) m[i][j] *= s;
        return *this;
    }

    constexpr Matrix& operator/=(T s) {
        for (std::size_t i = 0; i < R; ++i) for (std::size_t j = 0; j < C; ++j) m[i][j] /= s;
        return *this;
    }

    constexpr Matrix operator-() const {
        Matrix r;
        for (std::size_t i = 0; i < R; ++i) for (std::size_t j = 0; j < C; ++j) r.m[i][j] = -m[i][j];
        return r;
    }

    constexpr bool operator==(const Matrix& o) const {
        for (std::size_t i = 0; i < R; ++i)
            for (std::size_t j = 0; j < C; ++j) if (!(m[i][j] == o.m[i][j])) return false;
        return true;
    }

    constexpr bool operator!=(const Matrix& o) const { return !(*this == o); }
    constexpr T determinant() const {
        static_assert(R == C, "determinant requires a square matrix");
        if constexpr (R == 1) {
            return m[0][0];
        } else if constexpr (R == 2) {
            return m[0][0] * m[1][1] - m[0][1] * m[1][0];
        } else if constexpr (R == 3) {
            return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                   m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                   m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
        } else {
            T sum = T(0);
            for (std::size_t c = 0; c < R; ++c) {
                Matrix<T, R - 1, R - 1> minor{};
                std::size_t mi = 0;
                for (std::size_t r = 1; r < R; ++r) {
                    std::size_t mj = 0;
                    for (std::size_t jc = 0; jc < R; ++jc) {
                        if (jc == c) continue;
                        minor.m[mi][mj++] = m[r][jc];
                    }
                    ++mi;
                }
                T sign = (c % 2 == 0) ? T(1) : T(-1);
                sum += sign * m[0][c] * minor.determinant();
            }
            return sum;
        }
    }

    constexpr Matrix inverted() const {
        static_assert(R == C, "inverse requires a square matrix");
        if constexpr (R == 2) {
            T det = m[0][0] * m[1][1] - m[0][1] * m[1][0];
            Matrix r;
            r.m[0][0] = m[1][1];
            r.m[1][1] = m[0][0];
            r.m[0][1] = -m[0][1];
            r.m[1][0] = -m[1][0];
            return det != T(0) ? r / det : Matrix::identity();
        } else if constexpr (R == 3) {
            T det = determinant();
            Matrix r;
            r.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
            r.m[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]);
            r.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
            r.m[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]);
            r.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]);
            r.m[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]);
            r.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
            r.m[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]);
            r.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
            return det != T(0) ? r / det : Matrix::identity();
        } else {
            Matrix a = *this;
            Matrix inv = Matrix::identity();
            for (std::size_t i = 0; i < R; ++i) {
                std::size_t pivot = i;
                T best = a.m[i][i] < T(0) ? -a.m[i][i] : a.m[i][i];
                for (std::size_t r = i + 1; r < R; ++r) {
                    T v = a.m[r][i] < T(0) ? -a.m[r][i] : a.m[r][i];
                    if (v > best) {
                        best = v;
                        pivot = r;
                    }
                }
                if (best == T(0)) return Matrix::identity();
                if (pivot != i) {
                    for (std::size_t c = 0; c < R; ++c) {
                        T t = a.m[i][c];
                        a.m[i][c] = a.m[pivot][c];
                        a.m[pivot][c] = t;
                        t = inv.m[i][c];
                        inv.m[i][c] = inv.m[pivot][c];
                        inv.m[pivot][c] = t;
                    }
                }
                T d = a.m[i][i];
                for (std::size_t c = 0; c < R; ++c) {
                    a.m[i][c] /= d;
                    inv.m[i][c] /= d;
                }
                for (std::size_t r = 0; r < R; ++r) {
                    if (r == i) continue;
                    T f = a.m[r][i];
                    if (f == T(0)) continue;
                    for (std::size_t c = 0; c < R; ++c) {
                        a.m[r][c] -= f * a.m[i][c];
                        inv.m[r][c] -= f * inv.m[i][c];
                    }
                }
            }
            return inv;
        }
    }

    constexpr T max_abs() const {
        T best = T(0);
        for (std::size_t i = 0; i < R; ++i)
            for (std::size_t j = 0; j < C; ++j) {
                T v = m[i][j] < T(0) ? -m[i][j] : m[i][j];
                if (v > best) best = v;
            }
        return best;
    }
};

template <class T, std::size_t R, std::size_t C>
constexpr Matrix<T, R, C> operator+(Matrix<T, R, C> a, const Matrix<T, R, C>& b) {
    return a += b;
}
template <class T, std::size_t R, std::size_t C>
constexpr Matrix<T, R, C> operator-(Matrix<T, R, C> a, const Matrix<T, R, C>& b) {
    return a -= b;
}
template <class T, std::size_t R, std::size_t C>
constexpr Matrix<T, R, C> operator*(Matrix<T, R, C> a, T s) {
    return a *= s;
}
template <class T, std::size_t R, std::size_t C>
constexpr Matrix<T, R, C> operator*(T s, Matrix<T, R, C> a) {
    return a *= s;
}
template <class T, std::size_t R, std::size_t C>
constexpr Matrix<T, R, C> operator/(Matrix<T, R, C> a, T s) {
    return a /= s;
}

template <class T, std::size_t R, std::size_t K, std::size_t C>
constexpr Matrix<T, R, C> operator*(const Matrix<T, R, K>& a, const Matrix<T, K, C>& b) {
    Matrix<T, R, C> r;
    for (std::size_t i = 0; i < R; ++i)
        for (std::size_t k = 0; k < K; ++k) {
            T aik = a.m[i][k];
            if (aik == T(0)) continue;
            for (std::size_t j = 0; j < C; ++j) r.m[i][j] += aik * b.m[k][j];
        }
    return r;
}

template <class T, std::size_t R, std::size_t C>
constexpr Vector<T, R> operator*(const Matrix<T, R, C>& a, const Vector<T, C>& v) {
    Vector<T, R> r;
    for (std::size_t i = 0; i < R; ++i) {
        T s = T(0);
        for (std::size_t j = 0; j < C; ++j) s += a.m[i][j] * v[j];
        r[i] = s;
    }
    return r;
}

template <class T, std::size_t N>
constexpr Matrix<T, N, N> transpose(const Matrix<T, N, N>& m) {
    return m.transposed();
}

template <class T, std::size_t R, std::size_t C>
constexpr Matrix<T, C, R> transpose(const Matrix<T, R, C>& m) {
    return m.transposed();
}

template <class T, std::size_t N>
constexpr T determinant(const Matrix<T, N, N>& m) {
    return m.determinant();
}

template <class T, std::size_t N>
constexpr Matrix<T, N, N> inverse(const Matrix<T, N, N>& m) {
    return m.inverted();
}

template <class T, std::size_t R, std::size_t C>
constexpr bool almost_equal(const Matrix<T, R, C>& a, const Matrix<T, R, C>& b, T eps = T(1e-6)) {
    for (std::size_t i = 0; i < R; ++i) for (std::size_t j = 0; j < C; ++j) if (!almost_equal(a.m[i][j], b.m[i][j], eps)) return false;
    return true;
}

template <class T, std::size_t N>
constexpr Matrix<T, N, N> outer(const Vector<T, N>& a, const Vector<T, N>& b) {
    Matrix<T, N, N> r;
    for (std::size_t i = 0; i < N; ++i) for (std::size_t j = 0; j < N; ++j) r.m[i][j] = a[i] * b[j];
    return r;
}

template <class T, std::size_t R, std::size_t C>
constexpr Matrix<T, C, R> swap_rows(const Matrix<T, R, C>& m, std::size_t a, std::size_t b) {
    Matrix<T, R, C> r = m;
    for (std::size_t c = 0; c < C; ++c) {
        T t = r.m[a][c];
        r.m[a][c] = r.m[b][c];
        r.m[b][c] = t;
    }
    return r;
}

using Mat2 = Matrix<float, 2, 2>;
using Mat3 = Matrix<float, 3, 3>;
using Mat4 = Matrix<float, 4, 4>;
using DMat2 = Matrix<double, 2, 2>;
using DMat3 = Matrix<double, 3, 3>;
using DMat4 = Matrix<double, 4, 4>;
template <class T>
struct Matrix<T, 3, 3> {
    T m[3][3]{};
    constexpr Matrix() = default;
    constexpr Matrix(std::initializer_list<std::initializer_list<T>> il) {
        std::size_t r = 0;
        for (const auto& row_il : il) {
            if (r >= 3) break;
            std::size_t c = 0;
            for (const auto& v : row_il) {
                if (c >= 3) break;
                m[r][c++] = v;
            }
            ++r;
        }
    }

    static constexpr Matrix zero() { return Matrix(); }
    static constexpr Matrix identity() {
        return Matrix{{T(1), T(0), T(0)}, {T(0), T(1), T(0)}, {T(0), T(0), T(1)}};
    }

    static constexpr std::size_t rows() { return 3; }
    static constexpr std::size_t cols() { return 3; }
    constexpr T& operator()(std::size_t r, std::size_t c) { return m[r][c]; }
    constexpr const T& operator()(std::size_t r, std::size_t c) const { return m[r][c]; }
    constexpr T* operator[](std::size_t r) { return m[r]; }
    constexpr const T* operator[](std::size_t r) const { return m[r]; }
    constexpr T* data() { return &m[0][0]; }
    constexpr const T* data() const { return &m[0][0]; }
    constexpr Vector<T, 3> row(std::size_t r) const { return Vector<T, 3>(m[r][0], m[r][1], m[r][2]); }
    constexpr Vector<T, 3> col(std::size_t c) const { return Vector<T, 3>(m[0][c], m[1][c], m[2][c]); }
    constexpr void set_row(std::size_t r, const Vector<T, 3>& v) { m[r][0] = v.x; m[r][1] = v.y; m[r][2] = v.z; }
    constexpr void set_col(std::size_t c, const Vector<T, 3>& v) { m[0][c] = v.x; m[1][c] = v.y; m[2][c] = v.z; }
    constexpr Matrix transposed() const {
        return Matrix{{m[0][0], m[1][0], m[2][0]},
                      {m[0][1], m[1][1], m[2][1]},
                      {m[0][2], m[1][2], m[2][2]}};
    }

    constexpr T trace() const { return m[0][0] + m[1][1] + m[2][2]; }
    constexpr Matrix& operator+=(const Matrix& o) {
        for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) m[i][j] += o.m[i][j];
        return *this;
    }
    constexpr Matrix& operator-=(const Matrix& o) {
        for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) m[i][j] -= o.m[i][j];
        return *this;
    }
    constexpr Matrix& operator*=(T s) {
        for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) m[i][j] *= s;
        return *this;
    }
    constexpr Matrix& operator/=(T s) {
        for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) m[i][j] /= s;
        return *this;
    }
    constexpr Matrix operator-() const {
        return Matrix{{-m[0][0], -m[0][1], -m[0][2]},
                      {-m[1][0], -m[1][1], -m[1][2]},
                      {-m[2][0], -m[2][1], -m[2][2]}};
    }
    constexpr bool operator==(const Matrix& o) const {
        for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) if (!(m[i][j] == o.m[i][j])) return false;
        return true;
    }
    constexpr bool operator!=(const Matrix& o) const { return !(*this == o); }
    constexpr T determinant() const {
        return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
               m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
               m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    }

    constexpr Matrix inverted() const {
        T det = determinant();
        if (det == T(0)) return identity();
        Matrix r;
        r.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) / det;
        r.m[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) / det;
        r.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) / det;
        r.m[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) / det;
        r.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) / det;
        r.m[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) / det;
        r.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) / det;
        r.m[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) / det;
        r.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) / det;
        return r;
    }

    constexpr T max_abs() const {
        T best = T(0);
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) {
                T v = m[i][j] < T(0) ? -m[i][j] : m[i][j];
                if (v > best) best = v;
            }
        return best;
    }

    static constexpr Matrix scale(const Vector<T, 3>& s) {
        return Matrix{{s.x, T(0), T(0)}, {T(0), s.y, T(0)}, {T(0), T(0), s.z}};
    }
    static constexpr Matrix scale(T s) { return scale(Vector<T, 3>(s, s, s)); }
    static constexpr Matrix rotation_x(T a) {
        T c = std::cos(a), s = std::sin(a);
        return Matrix{{T(1), T(0), T(0)}, {T(0), c, -s}, {T(0), s, c}};
    }
    static constexpr Matrix rotation_y(T a) {
        T c = std::cos(a), s = std::sin(a);
        return Matrix{{c, T(0), s}, {T(0), T(1), T(0)}, {-s, T(0), c}};
    }
    static constexpr Matrix rotation_z(T a) {
        T c = std::cos(a), s = std::sin(a);
        return Matrix{{c, -s, T(0)}, {s, c, T(0)}, {T(0), T(0), T(1)}};
    }
    static constexpr Matrix rotation_axis(const Vector<T, 3>& axis, T a) {
        Vector<T, 3> n = normalize(axis);
        T c = std::cos(a), s = std::sin(a), t = T(1) - c;
        return Matrix{{t * n.x * n.x + c, t * n.x * n.y - s * n.z, t * n.x * n.z + s * n.y},
                      {t * n.x * n.y + s * n.z, t * n.y * n.y + c, t * n.y * n.z - s * n.x},
                      {t * n.x * n.z - s * n.y, t * n.y * n.z + s * n.x, t * n.z * n.z + c}};
    }
};

template <class T>
struct Matrix<T, 4, 4> {
    T m[4][4]{};
    constexpr Matrix() = default;
    constexpr Matrix(std::initializer_list<std::initializer_list<T>> il) {
        std::size_t r = 0;
        for (const auto& row_il : il) {
            if (r >= 4) break;
            std::size_t c = 0;
            for (const auto& v : row_il) {
                if (c >= 4) break;
                m[r][c++] = v;
            }
            ++r;
        }
    }

    static constexpr Matrix zero() { return Matrix(); }
    static constexpr Matrix identity() {
        return Matrix{{T(1), T(0), T(0), T(0)},
                      {T(0), T(1), T(0), T(0)},
                      {T(0), T(0), T(1), T(0)},
                      {T(0), T(0), T(0), T(1)}};
    }

    static constexpr std::size_t rows() { return 4; }
    static constexpr std::size_t cols() { return 4; }
    constexpr T& operator()(std::size_t r, std::size_t c) { return m[r][c]; }
    constexpr const T& operator()(std::size_t r, std::size_t c) const { return m[r][c]; }
    constexpr T* operator[](std::size_t r) { return m[r]; }
    constexpr const T* operator[](std::size_t r) const { return m[r]; }
    constexpr T* data() { return &m[0][0]; }
    constexpr const T* data() const { return &m[0][0]; }
    constexpr Vector<T, 4> row(std::size_t r) const {
        return Vector<T, 4>(m[r][0], m[r][1], m[r][2], m[r][3]);
    }
    constexpr Vector<T, 4> col(std::size_t c) const {
        return Vector<T, 4>(m[0][c], m[1][c], m[2][c], m[3][c]);
    }
    constexpr void set_row(std::size_t r, const Vector<T, 4>& v) {
        m[r][0] = v.x;
        m[r][1] = v.y;
        m[r][2] = v.z;
        m[r][3] = v.w;
    }
    constexpr void set_col(std::size_t c, const Vector<T, 4>& v) {
        m[0][c] = v.x;
        m[1][c] = v.y;
        m[2][c] = v.z;
        m[3][c] = v.w;
    }

    constexpr Matrix transposed() const {
        return Matrix{{m[0][0], m[1][0], m[2][0], m[3][0]},
                      {m[0][1], m[1][1], m[2][1], m[3][1]},
                      {m[0][2], m[1][2], m[2][2], m[3][2]},
                      {m[0][3], m[1][3], m[2][3], m[3][3]}};
    }

    constexpr T trace() const { return m[0][0] + m[1][1] + m[2][2] + m[3][3]; }
    constexpr Matrix& operator+=(const Matrix& o) {
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) m[i][j] += o.m[i][j];
        return *this;
    }
    constexpr Matrix& operator-=(const Matrix& o) {
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) m[i][j] -= o.m[i][j];
        return *this;
    }
    constexpr Matrix& operator*=(T s) {
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) m[i][j] *= s;
        return *this;
    }
    constexpr Matrix& operator/=(T s) {
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) m[i][j] /= s;
        return *this;
    }
    constexpr Matrix operator-() const {
        Matrix r;
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) r.m[i][j] = -m[i][j];
        return r;
    }
    constexpr bool operator==(const Matrix& o) const {
        for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) if (!(m[i][j] == o.m[i][j])) return false;
        return true;
    }
    constexpr bool operator!=(const Matrix& o) const { return !(*this == o); }
    constexpr T determinant() const { return cofactor_det(); }
    constexpr T minor_det(std::size_t skip_r, std::size_t skip_c) const {
        T sub[3][3];
        std::size_t ri = 0;
        for (std::size_t r = 0; r < 4; ++r) {
            if (r == skip_r) continue;
            std::size_t ci = 0;
            for (std::size_t c = 0; c < 4; ++c) {
                if (c == skip_c) continue;
                sub[ri][ci++] = m[r][c];
            }
            ++ri;
        }
        return sub[0][0] * (sub[1][1] * sub[2][2] - sub[1][2] * sub[2][1]) -
               sub[0][1] * (sub[1][0] * sub[2][2] - sub[1][2] * sub[2][0]) +
               sub[0][2] * (sub[1][0] * sub[2][1] - sub[1][1] * sub[2][0]);
    }

    constexpr T cofactor_det() const {
        T s0 = m[0][0] * m[1][1] - m[0][1] * m[1][0];
        T s1 = m[0][0] * m[1][2] - m[0][2] * m[1][0];
        T s2 = m[0][0] * m[1][3] - m[0][3] * m[1][0];
        T s3 = m[0][1] * m[1][2] - m[0][2] * m[1][1];
        T s4 = m[0][1] * m[1][3] - m[0][3] * m[1][1];
        T s5 = m[0][2] * m[1][3] - m[0][3] * m[1][2];
        T c5 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
        T c4 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
        T c3 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
        T c2 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
        T c1 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
        T c0 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
        return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
    }

    constexpr Matrix inverted() const {
        T det = determinant();
        if (det == T(0)) return identity();
        Matrix r;
        for (std::size_t i = 0; i < 4; ++i) {
            for (std::size_t j = 0; j < 4; ++j) {
                T sign = ((i + j) % 2 == 0) ? T(1) : T(-1);
                r.m[j][i] = sign * minor_det(i, j) / det;
            }
        }
        return r;
    }

    constexpr T max_abs() const {
        T best = T(0);
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j) {
                T v = m[i][j] < T(0) ? -m[i][j] : m[i][j];
                if (v > best) best = v;
            }
        return best;
    }

    static constexpr Matrix translation(const Vector<T, 3>& v) {
        Matrix r = identity();
        r.m[0][3] = v.x;
        r.m[1][3] = v.y;
        r.m[2][3] = v.z;
        return r;
    }
    static constexpr Matrix translation(T x, T y, T z) { return translation(Vector<T, 3>(x, y, z)); }
    static constexpr Matrix scale(const Vector<T, 3>& s) {
        Matrix r = identity();
        r.m[0][0] = s.x;
        r.m[1][1] = s.y;
        r.m[2][2] = s.z;
        return r;
    }
    static constexpr Matrix scale(T s) { return scale(Vector<T, 3>(s, s, s)); }
    static constexpr Matrix rotation_x(T a) {
        T c = std::cos(a), s = std::sin(a);
        Matrix r = identity();
        r.m[1][1] = c;
        r.m[1][2] = -s;
        r.m[2][1] = s;
        r.m[2][2] = c;
        return r;
    }
    static constexpr Matrix rotation_y(T a) {
        T c = std::cos(a), s = std::sin(a);
        Matrix r = identity();
        r.m[0][0] = c;
        r.m[0][2] = s;
        r.m[2][0] = -s;
        r.m[2][2] = c;
        return r;
    }
    static constexpr Matrix rotation_z(T a) {
        T c = std::cos(a), s = std::sin(a);
        Matrix r = identity();
        r.m[0][0] = c;
        r.m[0][1] = -s;
        r.m[1][0] = s;
        r.m[1][1] = c;
        return r;
    }
    static constexpr Matrix rotation_axis(const Vector<T, 3>& axis, T a) {
        return embed3(Matrix<T, 3, 3>::rotation_axis(axis, a));
    }
    static constexpr Matrix rotation_euler(T rx, T ry, T rz) {
        return rotation_z(rz) * rotation_y(ry) * rotation_x(rx);
    }
    static constexpr Matrix embed3(const Matrix<T, 3, 3>& m3) {
        Matrix r = identity();
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) r.m[i][j] = m3.m[i][j];
        return r;
    }
    static constexpr Matrix look_at(const Vector<T, 3>& eye, const Vector<T, 3>& center, const Vector<T, 3>& up) {
        Vector<T, 3> f = normalize(center - eye);
        Vector<T, 3> s = normalize(cross(f, up));
        Vector<T, 3> u = cross(s, f);
        Matrix r = identity();
        r.m[0][0] = s.x;
        r.m[0][1] = s.y;
        r.m[0][2] = s.z;
        r.m[1][0] = u.x;
        r.m[1][1] = u.y;
        r.m[1][2] = u.z;
        r.m[2][0] = -f.x;
        r.m[2][1] = -f.y;
        r.m[2][2] = -f.z;
        r.m[0][3] = -dot(s, eye);
        r.m[1][3] = -dot(u, eye);
        r.m[2][3] = dot(f, eye);
        return r;
    }
    static constexpr Matrix perspective(T fovy, T aspect, T znear, T zfar) {
        T f = T(1) / std::tan(fovy / T(2));
        Matrix r = zero();
        r.m[0][0] = f / aspect;
        r.m[1][1] = f;
        r.m[2][2] = (zfar + znear) / (znear - zfar);
        r.m[2][3] = (T(2) * zfar * znear) / (znear - zfar);
        r.m[3][2] = T(-1);
        return r;
    }
    static constexpr Matrix orthographic(T left, T right, T bottom, T top, T znear, T zfar) {
        Matrix r = identity();
        r.m[0][0] = T(2) / (right - left);
        r.m[1][1] = T(2) / (top - bottom);
        r.m[2][2] = T(-2) / (zfar - znear);
        r.m[0][3] = -(right + left) / (right - left);
        r.m[1][3] = -(top + bottom) / (top - bottom);
        r.m[2][3] = -(zfar + znear) / (zfar - znear);
        return r;
    }
};

template <class T>
constexpr Vector<T, 4> mul_point(const Matrix<T, 4, 4>& m, const Vector<T, 3>& p) {
    Vector<T, 4> r;
    r.x = m.m[0][0] * p.x + m.m[0][1] * p.y + m.m[0][2] * p.z + m.m[0][3];
    r.y = m.m[1][0] * p.x + m.m[1][1] * p.y + m.m[1][2] * p.z + m.m[1][3];
    r.z = m.m[2][0] * p.x + m.m[2][1] * p.y + m.m[2][2] * p.z + m.m[2][3];
    r.w = m.m[3][0] * p.x + m.m[3][1] * p.y + m.m[3][2] * p.z + m.m[3][3];
    return r;
}

template <class T>
constexpr Vector<T, 3> transform_point(const Matrix<T, 4, 4>& m, const Vector<T, 3>& p) {
    Vector<T, 4> r = mul_point(m, p);
    T w = r.w != T(0) ? r.w : T(1);
    return Vector<T, 3>(r.x / w, r.y / w, r.z / w);
}

template <class T>
constexpr Vector<T, 3> transform_vector(const Matrix<T, 4, 4>& m, const Vector<T, 3>& v) {
    return Vector<T, 3>(m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
                        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
                        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z);
}

template <class T>
constexpr Vector<T, 3> transform_normal(const Matrix<T, 4, 4>& m, const Vector<T, 3>& n) {
    Matrix<T, 4, 4> inv = m.inverted().transposed();
    return transform_vector(inv, n);
}
}
