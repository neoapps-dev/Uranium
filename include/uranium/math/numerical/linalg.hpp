#pragma once
#include "uranium/core/error.hpp"
#include "uranium/math/matrix.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>
namespace uranium {
namespace numerical {
template <class T>
class DynMatrix {
public:
    DynMatrix() = default;
    DynMatrix(std::size_t rows, std::size_t cols, T fill = T(0)): m_rows(rows), m_cols(cols), m_data(rows * cols, fill) {}
    DynMatrix(std::initializer_list<std::initializer_list<T>> il) {
        m_rows = il.size();
        m_cols = m_rows > 0 ? il.begin()->size() : 0;
        m_data.resize(m_rows * m_cols);
        std::size_t r = 0;
        for (const auto& row_il : il) {
            std::size_t c = 0;
            for (const auto& v : row_il) {
                if (c < m_cols) (*this)(r, c) = v;
                ++c;
            }
            ++r;
        }
    }

    static DynMatrix identity(std::size_t n) {
        DynMatrix r(n, n, T(0));
        for (std::size_t i = 0; i < n; ++i) r(i, i) = T(1);
        return r;
    }

    static DynMatrix diagonal(const std::vector<T>& values) {
        DynMatrix r(values.size(), values.size(), T(0));
        for (std::size_t i = 0; i < values.size(); ++i) r(i, i) = values[i];
        return r;
    }

    std::size_t rows() const { return m_rows; }
    std::size_t cols() const { return m_cols; }
    bool empty() const { return m_data.empty(); }
    T& operator()(std::size_t r, std::size_t c) { return m_data[r * m_cols + c]; }
    const T& operator()(std::size_t r, std::size_t c) const { return m_data[r * m_cols + c]; }
    T* operator[](std::size_t r) { return &m_data[r * m_cols]; }
    const T* operator[](std::size_t r) const { return &m_data[r * m_cols]; }
    T* data() { return m_data.data(); }
    const T* data() const { return m_data.data(); }
    std::size_t size() const { return m_data.size(); }
    std::vector<T> row(std::size_t r) const { return std::vector<T>(m_data.begin() + static_cast<std::ptrdiff_t>(r * m_cols), m_data.begin() + static_cast<std::ptrdiff_t>((r + 1) * m_cols)); }
    std::vector<T> col(std::size_t c) const {
        std::vector<T> out(m_rows);
        for (std::size_t r = 0; r < m_rows; ++r) out[r] = (*this)(r, c);
        return out;
    }

    void set_row(std::size_t r, const std::vector<T>& v) {
        for (std::size_t c = 0; c < m_cols && c < v.size(); ++c) (*this)(r, c) = v[c];
    }

    void set_col(std::size_t c, const std::vector<T>& v) {
        for (std::size_t r = 0; r < m_rows && r < v.size(); ++r) (*this)(r, c) = v[r];
    }

    DynMatrix transposed() const {
        DynMatrix r(m_cols, m_rows, T(0));
        for (std::size_t i = 0; i < m_rows; ++i) for (std::size_t j = 0; j < m_cols; ++j) r(j, i) = (*this)(i, j);
        return r;
    }

    DynMatrix& operator+=(const DynMatrix& o) {
        for (std::size_t i = 0; i < m_data.size(); ++i) m_data[i] += o.m_data[i];
        return *this;
    }
    DynMatrix& operator-=(const DynMatrix& o) {
        for (std::size_t i = 0; i < m_data.size(); ++i) m_data[i] -= o.m_data[i];
        return *this;
    }
    DynMatrix& operator*=(T s) {
        for (T& v : m_data) v *= s;
        return *this;
    }
    DynMatrix& operator/=(T s) {
        for (T& v : m_data) v /= s;
        return *this;
    }

    T frobenius_norm() const {
        T s = T(0);
        for (T v : m_data) s += v * v;
        return std::sqrt(s);
    }

    T max_abs() const {
        T best = T(0);
        for (T v : m_data) {
            T a = v < T(0) ? -v : v;
            if (a > best) best = a;
        }
        return best;
    }

    template <std::size_t R, std::size_t C>
    static DynMatrix from_fixed(const Matrix<T, R, C>& m) {
        DynMatrix r(R, C, T(0));
        for (std::size_t i = 0; i < R; ++i)
            for (std::size_t j = 0; j < C; ++j) r(i, j) = m.m[i][j];
        return r;
    }

    template <std::size_t R, std::size_t C>
    Matrix<T, R, C> to_fixed() const {
        static_assert(R > 0 && C > 0, "positive dims required");
        Matrix<T, R, C> out;
        for (std::size_t i = 0; i < R; ++i) for (std::size_t j = 0; j < C; ++j) out.m[i][j] = (i < m_rows && j < m_cols) ? (*this)(i, j) : T(0);
        return out;
    }

    bool operator==(const DynMatrix& o) const {
        return m_rows == o.m_rows && m_cols == o.m_cols && m_data == o.m_data;
    }

private:
    std::size_t m_rows = 0;
    std::size_t m_cols = 0;
    std::vector<T> m_data;
};

template <class T>
DynMatrix<T> operator+(DynMatrix<T> a, const DynMatrix<T>& b) {
    return a += b;
}
template <class T>
DynMatrix<T> operator-(DynMatrix<T> a, const DynMatrix<T>& b) {
    return a -= b;
}
template <class T>
DynMatrix<T> operator*(DynMatrix<T> a, T s) {
    return a *= s;
}
template <class T>
DynMatrix<T> operator*(T s, DynMatrix<T> a) {
    return a *= s;
}
template <class T>
DynMatrix<T> operator/(DynMatrix<T> a, T s) {
    return a /= s;
}

template <class T>
DynMatrix<T> operator*(const DynMatrix<T>& a, const DynMatrix<T>& b) {
    DynMatrix<T> r(a.rows(), b.cols(), T(0));
    for (std::size_t i = 0; i < a.rows(); ++i)
        for (std::size_t k = 0; k < a.cols(); ++k) {
            T aik = a(i, k);
            if (aik == T(0)) continue;
            for (std::size_t j = 0; j < b.cols(); ++j) r(i, j) += aik * b(k, j);
        }
    return r;
}

template <class T>
std::vector<T> operator*(const DynMatrix<T>& a, const std::vector<T>& v) {
    std::vector<T> r(a.rows(), T(0));
    for (std::size_t i = 0; i < a.rows(); ++i) {
        T s = T(0);
        for (std::size_t j = 0; j < a.cols(); ++j) s += a(i, j) * v[j];
        r[i] = s;
    }
    return r;
}

template <class T>
DynMatrix<T> transpose(const DynMatrix<T>& m) {
    return m.transposed();
}

template <class T>
T dot(const std::vector<T>& a, const std::vector<T>& b) {
    T s = T(0);
    std::size_t n = a.size() < b.size() ? a.size() : b.size();
    for (std::size_t i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

template <class T>
T norm(const std::vector<T>& a) {
    return std::sqrt(dot(a, a));
}

template <class T>
class LuDecomposition {
public:
    explicit LuDecomposition(const DynMatrix<T>& A): m_lu(A), m_pivot(A.rows()), m_sign(1), m_singular(false) {
        std::size_t n = A.rows();
        if (A.cols() != n) throw ValueError("lu decomposition requires a square matrix");
        for (std::size_t i = 0; i < n; ++i) m_pivot[i] = i;
        for (std::size_t k = 0; k < n; ++k) {
            std::size_t best = k;
            T best_val = abs_of(m_lu(k, k));
            for (std::size_t r = k + 1; r < n; ++r) {
                T v = abs_of(m_lu(r, k));
                if (v > best_val) {
                    best_val = v;
                    best = r;
                }
            }
            if (best_val == T(0)) {
                m_singular = true;
                continue;
            }
            if (best != k) {
                for (std::size_t c = 0; c < n; ++c) std::swap(m_lu(k, c), m_lu(best, c));
                std::swap(m_pivot[k], m_pivot[best]);
                m_sign = -m_sign;
            }
            T pivot = m_lu(k, k);
            for (std::size_t r = k + 1; r < n; ++r) {
                T factor = m_lu(r, k) / pivot;
                m_lu(r, k) = factor;
                if (factor == T(0)) continue;
                for (std::size_t c = k + 1; c < n; ++c) m_lu(r, c) -= factor * m_lu(k, c);
            }
        }
    }

    bool singular() const { return m_singular; }
    std::vector<T> solve(const std::vector<T>& b) const {
        std::size_t n = m_lu.rows();
        if (b.size() != n) throw ValueError("rhs size does not match matrix dimensions");
        if (m_singular) throw DomainError("matrix is singular");
        std::vector<T> x(n);
        for (std::size_t i = 0; i < n; ++i) x[i] = b[m_pivot[i]];
        for (std::size_t i = 1; i < n; ++i) {
            T s = x[i];
            for (std::size_t j = 0; j < i; ++j) s -= m_lu(i, j) * x[j];
            x[i] = s;
        }
        for (std::size_t i = n; i-- > 0;) {
            T s = x[i];
            for (std::size_t j = i + 1; j < n; ++j) s -= m_lu(i, j) * x[j];
            T d = m_lu(i, i);
            if (d == T(0)) throw DomainError("matrix is singular");
            x[i] = s / d;
        }
        return x;
    }

    T determinant() const {
        std::size_t n = m_lu.rows();
        T d = T(m_sign);
        for (std::size_t i = 0; i < n; ++i) d *= m_lu(i, i);
        return m_singular ? T(0) : d;
    }

    DynMatrix<T> inverse() const {
        std::size_t n = m_lu.rows();
        DynMatrix<T> inv(n, n, T(0));
        std::vector<T> e(n, T(0));
        for (std::size_t c = 0; c < n; ++c) {
            std::fill(e.begin(), e.end(), T(0));
            e[c] = T(1);
            std::vector<T> col = solve(e);
            for (std::size_t r = 0; r < n; ++r) inv(r, c) = col[r];
        }
        return inv;
    }

private:
    static T abs_of(T v) { return v < T(0) ? -v : v; }
    DynMatrix<T> m_lu;
    std::vector<std::size_t> m_pivot;
    int m_sign = 1;
    bool m_singular = false;
};

template <class T>
std::vector<T> solve(const DynMatrix<T>& A, const std::vector<T>& b) {
    return LuDecomposition<T>(A).solve(b);
}

template <class T>
T determinant(const DynMatrix<T>& A) {
    return LuDecomposition<T>(A).determinant();
}

template <class T>
DynMatrix<T> inverse(const DynMatrix<T>& A) {
    return LuDecomposition<T>(A).inverse();
}

struct EigenDecomposition {
    std::vector<double> eigenvalues;
    DynMatrix<double> eigenvectors;
    bool converged = false;
    std::pair<std::size_t, std::size_t> extremal() const {
        std::size_t lo = 0, hi = 0;
        for (std::size_t i = 0; i < eigenvalues.size(); ++i) {
            if (eigenvalues[i] < eigenvalues[lo]) lo = i;
            if (eigenvalues[i] > eigenvalues[hi]) hi = i;
        }
        return {lo, hi};
    }
};

inline EigenDecomposition eigen_symmetric(const DynMatrix<double>& A, int max_sweeps = 64) {
    std::size_t n = A.rows();
    if (A.cols() != n) throw ValueError("eigen_symmetric requires a square matrix");
    EigenDecomposition out;
    DynMatrix<double> a = A;
    DynMatrix<double> v = DynMatrix<double>::identity(n);
    out.eigenvalues.resize(n);
    for (int sweep = 0; sweep < max_sweeps; ++sweep) {
        double off = 0.0;
        for (std::size_t p = 0; p < n; ++p) for (std::size_t q = p + 1; q < n; ++q) off += a(p, q) * a(p, q);
        if (std::sqrt(off) < 1e-14) break;
        for (std::size_t p = 0; p + 1 < n; ++p) {
            for (std::size_t q = p + 1; q < n; ++q) {
                double apq = a(p, q);
                if (std::fabs(apq) < 1e-300) continue;
                double app = a(p, p), aqq = a(q, q);
                double theta = (aqq - app) / (2.0 * apq);
                double t = (theta >= 0.0 ? 1.0 : -1.0) / (std::fabs(theta) + std::sqrt(theta * theta + 1.0));
                double c = 1.0 / std::sqrt(t * t + 1.0);
                double s = t * c;
                for (std::size_t k = 0; k < n; ++k) {
                    if (k == p || k == q) continue;
                    double akp = a(k, p), akq = a(k, q);
                    a(k, p) = c * akp - s * akq;
                    a(p, k) = a(k, p);
                    a(k, q) = s * akp + c * akq;
                    a(q, k) = a(k, q);
                }
                a(p, p) = app - t * apq;
                a(q, q) = aqq + t * apq;
                a(p, q) = 0.0;
                a(q, p) = 0.0;
                for (std::size_t k = 0; k < n; ++k) {
                    double vkp = v(k, p), vkq = v(k, q);
                    v(k, p) = c * vkp - s * vkq;
                    v(k, q) = s * vkp + c * vkq;
                }
            }
        }
    }

    double off = 0.0;
    for (std::size_t p = 0; p < n; ++p) for (std::size_t q = p + 1; q < n; ++q) off += a(p, q) * a(p, q);
    out.converged = std::sqrt(off) < 1e-10;
    for (std::size_t i = 0; i < n; ++i) out.eigenvalues[i] = a(i, i);
    std::vector<std::size_t> order(n);
    for (std::size_t i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](std::size_t x, std::size_t y) { return out.eigenvalues[x] > out.eigenvalues[y]; });
    std::vector<double> sorted_vals(n);
    DynMatrix<double> sorted_vecs(n, n, 0.0);
    for (std::size_t j = 0; j < n; ++j) {
        sorted_vals[j] = out.eigenvalues[order[j]];
        for (std::size_t i = 0; i < n; ++i) sorted_vecs(i, j) = v(i, order[j]);
    }
    out.eigenvalues = std::move(sorted_vals);
    out.eigenvectors = std::move(sorted_vecs);
    return out;
}

struct QrDecomposition {
    DynMatrix<double> r;
    DynMatrix<double> householder;
    std::vector<double> tau;
    std::size_t m = 0;
    std::size_t n = 0;
    std::vector<double> q_transpose_mul(const std::vector<double>& b) const {
        std::vector<double> y = b;
        y.resize(m, 0.0);
        for (std::size_t k = 0; k < n; ++k) {
            if (tau[k] == 0.0) continue;
            double dotv = y[k];
            for (std::size_t i = k + 1; i < m; ++i) dotv += householder(i, k) * y[i];
            double fac = tau[k] * dotv;
            y[k] -= fac;
            for (std::size_t i = k + 1; i < m; ++i) y[i] -= fac * householder(i, k);
        }
        return y;
    }

    std::vector<double> back_substitute(const std::vector<double>& y) const {
        std::vector<double> x(n, 0.0);
        for (std::size_t i = n; i-- > 0;) {
            double s = y[i];
            for (std::size_t j = i + 1; j < n; ++j) s -= r(i, j) * x[j];
            if (r(i, i) == 0.0) throw DomainError("rank deficient matrix in qr solve");
            x[i] = s / r(i, i);
        }
        return x;
    }

    std::vector<double> solve(const std::vector<double>& b) const {
        return back_substitute(q_transpose_mul(b));
    }

    DynMatrix<double> explicit_q() const {
        DynMatrix<double> q = DynMatrix<double>::identity(m);
        for (std::size_t k = 0; k < n; ++k) {
            if (tau[k] == 0.0) continue;
            for (std::size_t i = 0; i < m; ++i) {
                double dotv = q(i, k);
                for (std::size_t j = k + 1; j < m; ++j) dotv += q(i, j) * householder(j, k);
                double fac = tau[k] * dotv;
                q(i, k) -= fac;
                for (std::size_t j = k + 1; j < m; ++j) q(i, j) -= fac * householder(j, k);
            }
        }
        return q;
    }
};

inline QrDecomposition qr(const DynMatrix<double>& A) {
    std::size_t m = A.rows(), n = A.cols();
    if (m < n) throw ValueError("qr requires m >= n");
    QrDecomposition out;
    out.r = A;
    out.m = m;
    out.n = n;
    out.tau.assign(n, 0.0);
    out.householder = DynMatrix<double>(m, n, 0.0);
    for (std::size_t k = 0; k < n; ++k) {
        double x1 = out.r(k, k);
        double normx = 0.0;
        for (std::size_t i = k; i < m; ++i) normx += out.r(i, k) * out.r(i, k);
        normx = std::sqrt(normx);
        if (normx == 0.0) continue;
        double beta = x1 >= 0.0 ? -normx : normx;
        double denom = x1 - beta;
        if (denom == 0.0) continue;
        double tau = (beta - x1) / beta;
        out.tau[k] = tau;
        for (std::size_t i = k + 1; i < m; ++i) {
            out.householder(i, k) = out.r(i, k) / denom;
            out.r(i, k) = 0.0;
        }
        out.r(k, k) = beta;
        for (std::size_t j = k + 1; j < n; ++j) {
            double dotv = out.r(k, j);
            for (std::size_t i = k + 1; i < m; ++i) dotv += out.householder(i, k) * out.r(i, j);
            double fac = tau * dotv;
            out.r(k, j) -= fac;
            for (std::size_t i = k + 1; i < m; ++i) out.r(i, j) -= fac * out.householder(i, k);
        }
    }
    return out;
}

inline std::vector<double> least_squares(const DynMatrix<double>& A, const std::vector<double>& b) {
    QrDecomposition decomposition = qr(A);
    return decomposition.solve(b);
}
}
}
