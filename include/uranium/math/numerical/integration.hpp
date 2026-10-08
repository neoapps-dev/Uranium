#pragma once
#include <cmath>
#include <cstddef>
#include <vector>
namespace uranium {
namespace numerical {
template <class F, class T>
T trapezoid(F f, T a, T b, int n) {
    if (n < 1) n = 1;
    T h = (b - a) / T(n);
    T s = (f(a) + f(b)) * T(0.5);
    for (int i = 1; i < n; ++i) s += f(a + h * T(i));
    return s * h;
}

template <class F, class T>
T simpson(F f, T a, T b, int n) {
    if (n < 1) n = 1;
    if (n % 2 == 1) ++n;
    T h = (b - a) / T(n);
    T s = f(a) + f(b);
    for (int i = 1; i < n; ++i) {
        T w = (i % 2 == 1) ? T(4) : T(2);
        s += w * f(a + h * T(i));
    }
    return s * h / T(3);
}

template <class F, class T>
T gauss_legendre(F f, T a, T b, int panels = 8) {
    static const double nodes[3] = {0.0, 0.5384693101056831, -0.5384693101056831};
    static const double weights[3] = {0.5688888888888889, 0.4786286704993665, 0.4786286704993665};
    static const double ext_nodes[2] = {0.9061798459386640, -0.9061798459386640};
    static const double ext_weights[2] = {0.2369268850561891, 0.2369268850561891};
    if (panels < 1) panels = 1;
    T total = T(0);
    T half = (b - a) / T(panels) * T(0.5);
    for (int p = 0; p < panels; ++p) {
        T c = a + (T(p) + T(0.5)) * ((b - a) / T(panels));
        for (int i = 0; i < 3; ++i) total += T(weights[i]) * f(c + T(nodes[i]) * half);
        for (int i = 0; i < 2; ++i) total += T(ext_weights[i]) * f(c + T(ext_nodes[i]) * half);
    }
    return total * half;
}

template <class F, class T>
T adaptive_simpson(F f, T a, T b, T tol = T(1e-10), int max_depth = 40) {
    struct Helper {
        F* fn;
        int depth;
        T tol;
        T recurse(T x0, T x1, T f0, T fm, T f1, T whole, T eps, int depth) {
            T xm = (x0 + x1) * T(0.5);
            T lm = (x0 + xm) * T(0.5);
            T rm = (xm + x1) * T(0.5);
            T flm = (*fn)(lm);
            T frm = (*fn)(rm);
            T left = (xm - x0) * (f0 + T(4) * flm + fm) / T(6);
            T right = (x1 - xm) * (fm + T(4) * frm + f1) / T(6);
            T delta = left + right - whole;
            if (depth <= 0 || std::fabs(delta) <= T(15) * eps) return left + right + delta / T(15);
            return recurse(x0, xm, f0, flm, fm, left, eps * T(0.5), depth - 1) + recurse(xm, x1, fm, frm, f1, right, eps * T(0.5), depth - 1);
        }
    };
    if (a == b) return T(0);
    Helper h{&f, max_depth, tol};
    T f0 = f(a), f1 = f(b), fm = f((a + b) * T(0.5));
    T whole = (b - a) * (f0 + T(4) * fm + f1) / T(6);
    return h.recurse(a, b, f0, fm, f1, whole, tol, max_depth);
}

template <class T>
T trapezoid_samples(const std::vector<T>& ys, T h) {
    if (ys.size() < 2) return T(0);
    T s = (ys.front() + ys.back()) * T(0.5);
    for (std::size_t i = 1; i + 1 < ys.size(); ++i) s += ys[i];
    return s * h;
}

template <class T>
T simpson_samples(const std::vector<T>& ys, T h) {
    std::size_t n = ys.size();
    if (n < 2) return T(0);
    if (n == 2) return (ys[0] + ys[1]) * h * T(0.5);
    std::size_t m = (n % 2 == 0) ? n - 1 : n;
    T s = ys[0] + ys[m - 1];
    for (std::size_t i = 1; i + 1 < m; ++i) s += (i % 2 == 1) ? T(4) * ys[i] : T(2) * ys[i];
    T result = s * h / T(3);
    if (n % 2 == 0) result += (ys[n - 2] + ys[n - 1]) * h * T(0.5);
    return result;
}

template <class F, class T>
T derivative(F f, T x, T h = T(1e-5)) {
    return (f(x + h) - f(x - h)) / (T(2) * h);
}

template <class F, class T>
T derivative2(F f, T x, T h = T(1e-4)) {
    return (f(x + h) - T(2) * f(x) + f(x - h)) / (h * h);
}
}
}
