#pragma once
#include "uranium/math/functions.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <vector>
namespace uranium {
namespace numerical {
struct OptOptions {
    double tolerance = 1e-10;
    int max_iterations = 500;
};

struct OptResult {
    std::vector<double> x;
    double value = 0.0;
    int iterations = 0;
    bool converged = false;
};

template <class F, class T>
T golden_section(F f, T a, T b, T tol = T(1e-8), int max_iterations = 200) {
    const T inv_phi = (T(1) - std::sqrt(T(5))) / T(2);
    T c = b - (b - a) * (T(1) - inv_phi);
    T d = a + (b - a) * (T(1) - inv_phi);
    T fc = f(c), fd = f(d);
    T lo = a, hi = b;
    int i = 0;
    for (; i < max_iterations && std::fabs(b - a) > tol * (std::fabs(a) + std::fabs(b) + T(1e-30)); ++i) {
        if (fc < fd) {
            hi = d;
            d = c;
            fd = fc;
            c = b - (b - a) * (T(1) - inv_phi);
            fc = f(c);
        } else {
            lo = c;
            c = d;
            fc = fd;
            d = a + (b - a) * (T(1) - inv_phi);
            fd = f(d);
        }
    }
    return fc < fd ? c : d;
}

using ScalarFn = std::function<double(const std::vector<double>&)>;
using GradientFn = std::function<std::vector<double>(const std::vector<double>&)>;
struct GradientDescentOptions : OptOptions {
    double learning_rate = 0.1;
    bool backtracking = true;
    double backtracking_c = 1e-4;
    double backtracking_shrink = 0.5;
    int backtracking_max = 40;
    double gradient_tolerance = 1e-8;
};

inline OptResult gradient_descent(ScalarFn f, GradientFn grad, std::vector<double> x0, GradientDescentOptions opts = {}) {
    OptResult r;
    r.x = std::move(x0);
    double fx = f(r.x);
    for (int it = 0; it < opts.max_iterations; ++it) {
        r.iterations = it + 1;
        std::vector<double> g = grad(r.x);
        double gnorm = 0.0;
        for (double v : g) gnorm += v * v;
        gnorm = std::sqrt(gnorm);
        if (gnorm < opts.gradient_tolerance) {
            r.converged = true;
            r.value = fx;
            return r;
        }
        double step = opts.learning_rate;
        if (opts.backtracking) {
            double slope = 0.0;
            for (std::size_t i = 0; i < g.size(); ++i) slope += g[i] * g[i];
            for (int k = 0; k < opts.backtracking_max; ++k) {
                std::vector<double> xn(r.x.size());
                for (std::size_t i = 0; i < xn.size(); ++i) xn[i] = r.x[i] - step * g[i];
                double fn = f(xn);
                if (fn <= fx - opts.backtracking_c * step * slope) break;
                step *= opts.backtracking_shrink;
                if (step < 1e-16) break;
            }
        }
        std::vector<double> xn(r.x.size());
        for (std::size_t i = 0; i < xn.size(); ++i) xn[i] = r.x[i] - step * g[i];
        double fn = f(xn);
        double dx = 0.0;
        for (std::size_t i = 0; i < xn.size(); ++i) dx += (xn[i] - r.x[i]) * (xn[i] - r.x[i]);
        r.x = std::move(xn);
        double prev = fx;
        fx = fn;
        if (std::sqrt(dx) < opts.tolerance && std::fabs(prev - fx) < opts.tolerance) {
            r.converged = true;
            break;
        }
    }
    r.value = fx;
    return r;
}

struct NelderMeadOptions : OptOptions {
    double initial_step = 0.5;
    double reflection = 1.0;
    double expansion = 2.0;
    double contraction = 0.5;
    double shrink = 0.5;
};

inline OptResult nelder_mead(ScalarFn f, std::vector<double> x0, NelderMeadOptions opts = {}) {
    OptResult r;
    std::size_t n = x0.size();
    if (n == 0) return r;
    std::vector<std::vector<double>> simplex(n + 1, x0);
    std::vector<double> values(n + 1);
    for (std::size_t i = 0; i < n; ++i) {
        double step = opts.initial_step * (std::fabs(x0[i]) > 1e-8 ? std::fabs(x0[i]) : 1.0);
        simplex[i + 1][i] += step;
    }
    for (std::size_t i = 0; i <= n; ++i) values[i] = f(simplex[i]);
    auto argsort = [&]() {
        std::vector<std::size_t> order(n + 1);
        for (std::size_t i = 0; i <= n; ++i) order[i] = i;
        std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return values[a] < values[b]; });
        return order;
    };

    for (int it = 0; it < opts.max_iterations; ++it) {
        r.iterations = it + 1;
        auto order = argsort();
        std::vector<std::size_t> idx = order;
        double best = values[idx[0]];
        double worst = values[idx[n]];
        double second_worst = values[idx[n - 1]];
        std::vector<double> centroid(n, 0.0);
        for (std::size_t i = 0; i < n; ++i) for (std::size_t d = 0; d < n; ++d) centroid[d] += simplex[idx[i]][d];
        for (std::size_t d = 0; d < n; ++d) centroid[d] /= static_cast<double>(n);
        auto point_at = [&](double scale) {
            std::vector<double> p(n);
            for (std::size_t d = 0; d < n; ++d) p[d] = centroid[d] + scale * (centroid[d] - simplex[idx[n]][d]);
            return p;
        };

        std::vector<double> xr = point_at(opts.reflection);
        double fr = f(xr);
        if (fr < best) {
            std::vector<double> xe(n);
            for (std::size_t d = 0; d < n; ++d) xe[d] = centroid[d] + opts.expansion * (xr[d] - centroid[d]);
            double fe = f(xe);
            if (fe < fr) {
                simplex[idx[n]] = xe;
                values[idx[n]] = fe;
            } else {
                simplex[idx[n]] = xr;
                values[idx[n]] = fr;
            }
        } else if (fr < second_worst) {
            simplex[idx[n]] = xr;
            values[idx[n]] = fr;
        } else {
            std::vector<double> xc(n);
            bool inside = fr < worst;
            for (std::size_t d = 0; d < n; ++d) xc[d] = centroid[d] + (inside ? opts.contraction : -opts.contraction) * (simplex[idx[n]][d] - centroid[d]);
            double fc = f(xc);
            if (fc < (inside ? fr : worst)) {
                simplex[idx[n]] = xc;
                values[idx[n]] = fc;
            } else {
                for (std::size_t i = 1; i <= n; ++i) {
                    for (std::size_t d = 0; d < n; ++d) simplex[idx[i]][d] = simplex[idx[0]][d] + opts.shrink * (simplex[idx[i]][d] - simplex[idx[0]][d]);
                    values[idx[i]] = f(simplex[idx[i]]);
                }
            }
        }

        std::vector<std::size_t> sorted = argsort();
        double span = std::fabs(values[sorted[n]] - values[sorted[0]]);
        if (span < opts.tolerance * (std::fabs(values[sorted[0]]) + 1.0)) {
            r.converged = true;
            break;
        }
        (void)worst;
    }
    auto order = argsort();
    r.x = simplex[order[0]];
    r.value = values[order[0]];
    return r;
}
}
}
