#pragma once
#include <cmath>
#include <cstddef>
#include <functional>
namespace uranium {
namespace numerical {
template <class T>
struct RootResult {
    T value{};
    T residual{};
    int iterations = 0;
    bool converged = false;
    explicit operator bool() const { return converged; }
};

struct RootOptions {
    double tolerance = 1e-12;
    int max_iterations = 256;
};

template <class T, class F>
RootResult<T> bisection(F f, T a, T b, RootOptions opts = {}) {
    RootResult<T> r;
    T fa = f(a), fb = f(b);
    if (fa == T(0)) {
        r.value = a;
        r.residual = T(0);
        r.converged = true;
        return r;
    }
    if (fb == T(0)) {
        r.value = b;
        r.residual = T(0);
        r.converged = true;
        return r;
    }
    if (fa * fb > T(0)) {
        r.value = a;
        r.residual = fa;
        return r;
    }
    T lo = a, hi = b, mid = a;
    for (int i = 0; i < opts.max_iterations; ++i) {
        mid = lo + (hi - lo) * T(0.5);
        T fm = f(mid);
        r.iterations = i + 1;
        r.residual = fm;
        if (fm == T(0) || (hi - lo) * T(0.5) < T(opts.tolerance)) {
            r.value = mid;
            r.converged = true;
            return r;
        }
        if (fa * fm < T(0)) {
            hi = mid;
        } else {
            lo = mid;
            fa = fm;
        }
    }
    r.value = mid;
    return r;
}

template <class T, class F, class D>
RootResult<T> newton(F f, D df, T x0, RootOptions opts = {}) {
    RootResult<T> r;
    T x = x0;
    for (int i = 0; i < opts.max_iterations; ++i) {
        T fx = f(x);
        T dfx = df(x);
        r.iterations = i + 1;
        r.residual = fx;
        if (std::fabs(fx) < T(opts.tolerance)) {
            r.value = x;
            r.converged = true;
            return r;
        }
        if (dfx == T(0)) break;
        T xn = x - fx / dfx;
        T step = std::fabs(xn - x);
        x = xn;
        if (step < T(opts.tolerance)) {
            r.value = x;
            r.residual = f(x);
            r.converged = true;
            return r;
        }
    }
    r.value = x;
    return r;
}

template <class T, class F>
RootResult<T> secant(F f, T x0, T x1, RootOptions opts = {}) {
    RootResult<T> r;
    T f0 = f(x0), f1 = f(x1);
    for (int i = 0; i < opts.max_iterations; ++i) {
        r.iterations = i + 1;
        if (f1 - f0 == T(0)) break;
        T x2 = x1 - f1 * (x1 - x0) / (f1 - f0);
        x0 = x1;
        f0 = f1;
        x1 = x2;
        f1 = f(x1);
        r.residual = f1;
        if (std::fabs(f1) < T(opts.tolerance) || std::fabs(x1 - x0) < T(opts.tolerance)) {
            r.value = x1;
            r.converged = true;
            return r;
        }
    }
    r.value = x1;
    return r;
}

template <class T, class F>
RootResult<T> brent(F f, T a, T b, RootOptions opts = {}) {
    RootResult<T> r;
    T fa = f(a), fb = f(b);
    if (fa == T(0)) {
        r.value = a;
        r.converged = true;
        return r;
    }
    if (fb == T(0)) {
        r.value = b;
        r.converged = true;
        return r;
    }
    if (fa * fb > T(0)) {
        r.value = a;
        r.residual = fa;
        return r;
    }
    T c = a, fc = fa;
    T d = b - a, e = d;
    for (int i = 0; i < opts.max_iterations; ++i) {
        r.iterations = i + 1;
        if (fb * fc > T(0)) {
            c = a;
            fc = fa;
            d = b - a;
            e = d;
        }
        if (std::fabs(fc) < std::fabs(fb)) {
            a = b;
            b = c;
            c = a;
            fa = fb;
            fb = fc;
            fc = fa;
        }
        T tol = T(2) * T(opts.tolerance) * std::fabs(b) + T(opts.tolerance) * T(0.5);
        T m = (c - b) * T(0.5);
        if (std::fabs(m) <= tol || fb == T(0)) {
            r.value = b;
            r.residual = fb;
            r.converged = true;
            return r;
        }
        if (std::fabs(e) >= tol && std::fabs(fa) > std::fabs(fb)) {
            T s = fb / fa;
            T p, q;
            if (a == c) {
                p = T(2) * m * s;
                q = T(1) - s;
            } else {
                T qq = fa / fc;
                T rr = fb / fc;
                p = s * (T(2) * m * qq * (qq - rr) - (b - a) * (rr - T(1)));
                q = (qq - T(1)) * (rr - T(1)) * (s - T(1));
            }
            if (p > T(0)) q = -q;
            p = std::fabs(p);
            if (T(2) * p < std::min(T(3) * m * q - std::fabs(tol * q), std::fabs(e * q))) {
                e = d;
                d = p / q;
            } else {
                d = m;
                e = m;
            }
        } else {
            d = m;
            e = m;
        }
        a = b;
        fa = fb;
        if (std::fabs(d) > tol) {
            b += d;
        } else {
            b += m > T(0) ? tol : -tol;
        }
        fb = f(b);
    }
    r.value = b;
    r.residual = fb;
    return r;
}
}
}
