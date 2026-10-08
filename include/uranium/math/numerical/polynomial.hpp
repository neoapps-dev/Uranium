#pragma once
#include "uranium/core/error.hpp"
#include "uranium/math/complex.hpp"
#include "uranium/math/numerical/linalg.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>
namespace uranium {
namespace numerical {
template <class T>
class Polynomial {
public:
    Polynomial() = default;
    Polynomial(std::initializer_list<T> coefficients) : m_coeffs(coefficients) { trim(); }
    Polynomial(std::vector<T> coefficients) : m_coeffs(std::move(coefficients)) { trim(); }
    static Polynomial constant(T c) { return Polynomial(std::vector<T>{c}); }
    static Polynomial x() { return Polynomial(std::vector<T>{T(0), T(1)}); }
    static Polynomial from_roots(const std::vector<Complex<T>>& roots) {
        Polynomial<Complex<T>> acc(std::vector<Complex<T>>{Complex<T>(T(1))});
        for (const auto& r : roots) {
            std::vector<Complex<T>> factor{Complex<T>(T(0)) - r, Complex<T>(T(1))};
            acc = acc * Polynomial<Complex<T>>(std::move(factor));
        }
        std::vector<T> out(acc.size());
        for (std::size_t i = 0; i < acc.size(); ++i) out[i] = acc.coefficients()[i].re;
        return Polynomial(std::move(out));
    }

    const std::vector<T>& coefficients() const { return m_coeffs; }
    std::size_t size() const { return m_coeffs.size(); }
    bool empty() const { return m_coeffs.empty(); }
    std::size_t degree() const {
        return m_coeffs.empty() ? 0 : m_coeffs.size() - 1;
    }

    bool is_zero() const {
        for (T c : m_coeffs) if (c != T(0)) return false;
        return true;
    }

    template <class U>
    U operator()(U x) const {
        if (m_coeffs.empty()) return U(0);
        U acc = static_cast<U>(m_coeffs.back());
        for (std::size_t i = m_coeffs.size() - 1; i-- > 0;) acc = acc * x + static_cast<U>(m_coeffs[i]);
        return acc;
    }

    T at(T x) const { return (*this)(x); }
    Polynomial derivative() const {
        if (m_coeffs.size() <= 1) return Polynomial(std::vector<T>{T(0)});
        std::vector<T> out(m_coeffs.size() - 1);
        for (std::size_t i = 1; i < m_coeffs.size(); ++i) out[i - 1] = m_coeffs[i] * static_cast<T>(i);
        return Polynomial(std::move(out));
    }

    Polynomial integral(T constant = T(0)) const {
        std::vector<T> out(m_coeffs.size() + 1, T(0));
        out[0] = constant;
        for (std::size_t i = 0; i < m_coeffs.size(); ++i) out[i + 1] = m_coeffs[i] / static_cast<T>(i + 1);
        return Polynomial(std::move(out));
    }

    Polynomial operator+(const Polynomial& o) const {
        std::vector<T> out(std::max(m_coeffs.size(), o.m_coeffs.size()), T(0));
        for (std::size_t i = 0; i < m_coeffs.size(); ++i) out[i] += m_coeffs[i];
        for (std::size_t i = 0; i < o.m_coeffs.size(); ++i) out[i] += o.m_coeffs[i];
        return Polynomial(std::move(out));
    }

    Polynomial operator-(const Polynomial& o) const {
        std::vector<T> out(std::max(m_coeffs.size(), o.m_coeffs.size()), T(0));
        for (std::size_t i = 0; i < m_coeffs.size(); ++i) out[i] += m_coeffs[i];
        for (std::size_t i = 0; i < o.m_coeffs.size(); ++i) out[i] -= o.m_coeffs[i];
        return Polynomial(std::move(out));
    }

    Polynomial operator*(const Polynomial& o) const {
        if (m_coeffs.empty() || o.m_coeffs.empty()) return Polynomial(std::vector<T>{T(0)});
        std::vector<T> out(m_coeffs.size() + o.m_coeffs.size() - 1, T(0));
        for (std::size_t i = 0; i < m_coeffs.size(); ++i) for (std::size_t j = 0; j < o.m_coeffs.size(); ++j) out[i + j] += m_coeffs[i] * o.m_coeffs[j];
        return Polynomial(std::move(out));
    }

    Polynomial operator*(T s) const {
        std::vector<T> out = m_coeffs;
        for (T& c : out) c *= s;
        return Polynomial(std::move(out));
    }

    Polynomial& operator+=(const Polynomial& o) { return *this = *this + o; }
    Polynomial& operator-=(const Polynomial& o) { return *this = *this - o; }
    Polynomial& operator*=(const Polynomial& o) { return *this = *this * o; }
    std::vector<Complex<T>> roots(T tol = T(1e-12), int max_iterations = 500) const {
        Polynomial<T> p = *this;
        p.trim_leading();
        std::size_t n = p.m_coeffs.size();
        if (n <= 1) return {};
        T lead = p.m_coeffs.back();
        for (T& c : p.m_coeffs) c /= lead;
        std::vector<Complex<T>> r(n - 1);
        Complex<T> base(T(0.4), T(0.9));
        Complex<T> cur(T(1), T(0));
        for (std::size_t i = 0; i < r.size(); ++i) {
            r[i] = cur;
            cur = cur * base;
            cur = Complex<T>(cur.re + T(0.1), cur.im + T(0.05));
        }
        for (int it = 0; it < max_iterations; ++it) {
            T max_delta = T(0);
            for (std::size_t i = 0; i < r.size(); ++i) {
                Complex<T> num = p.eval_complex(r[i]);
                Complex<T> den(T(1), T(0));
                for (std::size_t j = 0; j < r.size(); ++j) {
                    if (i == j) continue;
                    den = den * (r[i] - r[j]);
                }
                if (den.abs() < T(1e-300)) continue;
                Complex<T> delta = num / den;
                r[i] = r[i] - delta;
                max_delta = std::max(max_delta, delta.abs());
            }
            if (max_delta < tol) break;
        }
        return r;
    }

    std::vector<T> real_roots(T tol = T(1e-9)) const {
        auto cs = roots();
        std::vector<T> out;
        for (const auto& c : cs) if (std::fabs(c.im) < tol) out.push_back(c.re);
        std::sort(out.begin(), out.end());
        return out;
    }

    void trim() {
        while (m_coeffs.size() > 1 && m_coeffs.back() == T(0)) m_coeffs.pop_back();
    }

private:
    void trim_leading() {
        while (m_coeffs.size() > 1 && m_coeffs.back() == T(0)) m_coeffs.pop_back();
    }

    Complex<T> eval_complex(const Complex<T>& x) const {
        if (m_coeffs.empty()) return Complex<T>(T(0), T(0));
        Complex<T> acc(static_cast<T>(m_coeffs.back()), T(0));
        for (std::size_t i = m_coeffs.size() - 1; i-- > 0;) acc = acc * x + Complex<T>(m_coeffs[i], T(0));
        return acc;
    }

    std::vector<T> m_coeffs;
};

template <class T>
Polynomial<T> operator*(T s, const Polynomial<T>& p) {
    return p * s;
}

inline Polynomial<double> polynomial_fit(const std::vector<double>& xs, const std::vector<double>& ys, std::size_t degree) {
    std::size_t n = xs.size();
    std::size_t d = degree + 1;
    if (n < d) throw ValueError("not enough points for polynomial fit");
    DynMatrix<double> A(n, d, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        double pow = 1.0;
        for (std::size_t j = 0; j < d; ++j) {
            A(i, j) = pow;
            pow *= xs[i];
        }
    }
    std::vector<double> coeffs = least_squares(A, ys);
    return Polynomial<double>(std::move(coeffs));
}
}
}
