#pragma once
#include "uranium/core/error.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <vector>
namespace uranium {
namespace numerical {
using State = std::vector<double>;
using OdeFn = std::function<State(double, const State&)>;
struct OdeSolution {
    std::vector<double> t;
    std::vector<State> y;
    std::size_t size() const { return t.size(); }
    bool empty() const { return t.empty(); }
    double final_time() const { return t.empty() ? 0.0 : t.back(); }
    State at(double time) const {
        if (y.empty()) return State();
        if (y.size() == 1) return y[0];
        if (time <= t.front()) return y.front();
        if (time >= t.back()) return y.back();
        std::size_t hi = 1;
        while (hi + 1 < t.size() && t[hi] < time) ++hi;
        std::size_t lo = hi - 1;
        double span = t[hi] - t[lo];
        double u = span > 0.0 ? (time - t[lo]) / span : 0.0;
        State out(y[lo].size());
        for (std::size_t i = 0; i < out.size(); ++i) out[i] = y[lo][i] + (y[hi][i] - y[lo][i]) * u;
        return out;
    }

    std::vector<double> series(std::size_t component) const {
        std::vector<double> out;
        out.reserve(y.size());
        for (const auto& s : y) out.push_back(component < s.size() ? s[component] : 0.0);
        return out;
    }
};

struct OdeOptions {
    double tolerance = 1e-8;
    double initial_step = 0.0;
    double min_step = 1e-12;
    double max_step = 1e30;
    double max_steps = 1e6;
};

namespace detail {
inline State add_scaled(const State& a, const State& b, double s) {
    State r(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) r[i] = a[i] + b[i] * s;
    return r;
}

inline State axpy(double a, const State& x, const State& y) {
    State r(x.size());
    for (std::size_t i = 0; i < x.size(); ++i) r[i] = a * x[i] + y[i];
    return r;
}
}

inline OdeSolution euler(const OdeFn& f, State y0, double t0, double t1, double dt) {
    OdeSolution sol;
    if (dt <= 0.0) throw ValueError("ode step size must be positive");
    sol.t.push_back(t0);
    sol.y.push_back(y0);
    double t = t0;
    State y = std::move(y0);
    while (t < t1) {
        double h = std::min(dt, t1 - t);
        State k = f(t, y);
        y = detail::add_scaled(y, k, h);
        t += h;
        sol.t.push_back(t);
        sol.y.push_back(y);
    }
    return sol;
}

inline OdeSolution midpoint(const OdeFn& f, State y0, double t0, double t1, double dt) {
    OdeSolution sol;
    if (dt <= 0.0) throw ValueError("ode step size must be positive");
    sol.t.push_back(t0);
    sol.y.push_back(y0);
    double t = t0;
    State y = std::move(y0);
    while (t < t1) {
        double h = std::min(dt, t1 - t);
        State k1 = f(t, y);
        State ym = detail::add_scaled(y, k1, h * 0.5);
        State k2 = f(t + h * 0.5, ym);
        for (std::size_t i = 0; i < y.size(); ++i) y[i] += h * k2[i];
        t += h;
        sol.t.push_back(t);
        sol.y.push_back(y);
    }
    return sol;
}

inline OdeSolution rk4(const OdeFn& f, State y0, double t0, double t1, double dt) {
    OdeSolution sol;
    if (dt <= 0.0) throw ValueError("ode step size must be positive");
    sol.t.push_back(t0);
    sol.y.push_back(y0);
    double t = t0;
    State y = std::move(y0);
    std::size_t n = y.size();
    State k1(n), k2(n), k3(n), k4(n);
    while (t < t1) {
        double h = std::min(dt, t1 - t);
        k1 = f(t, y);
        k2 = f(t + h * 0.5, detail::add_scaled(y, k1, h * 0.5));
        k3 = f(t + h * 0.5, detail::add_scaled(y, k2, h * 0.5));
        k4 = f(t + h, detail::add_scaled(y, k3, h));
        for (std::size_t i = 0; i < n; ++i) y[i] += (h / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
        t += h;
        sol.t.push_back(t);
        sol.y.push_back(y);
    }
    return sol;
}

inline OdeSolution rk45(const OdeFn& f, State y0, double t0, double t1, OdeOptions opts = {}) {
    OdeSolution sol;
    if (t1 <= t0) {
        sol.t.push_back(t0);
        sol.y.push_back(y0);
        return sol;
    }
    const double c2 = 1.0 / 5.0, c3 = 3.0 / 10.0, c4 = 4.0 / 5.0, c5 = 8.0 / 9.0;
    const double a21 = 1.0 / 5.0;
    const double a31 = 3.0 / 40.0, a32 = 9.0 / 40.0;
    const double a41 = 44.0 / 45.0, a42 = -56.0 / 15.0, a43 = 32.0 / 9.0;
    const double a51 = 19372.0 / 6561.0, a52 = -25360.0 / 2187.0, a53 = 64448.0 / 6561.0, a54 = -212.0 / 729.0;
    const double a61 = 9017.0 / 3168.0, a62 = -355.0 / 33.0, a63 = 46732.0 / 5247.0, a64 = 49.0 / 176.0, a65 = -5103.0 / 18656.0;
    const double a71 = 35.0 / 384.0, a73 = 500.0 / 1113.0, a74 = 125.0 / 192.0, a75 = -2187.0 / 6784.0, a76 = 11.0 / 84.0;
    const double e1 = 5179.0 / 57600.0 - 35.0 / 384.0;
    const double e3 = 7571.0 / 16695.0 - 500.0 / 1113.0;
    const double e4 = 393.0 / 640.0 - 125.0 / 192.0;
    const double e5 = -92097.0 / 339200.0 + 2187.0 / 6784.0;
    const double e6 = 187.0 / 2100.0 - 11.0 / 84.0;
    const double e7 = 1.0 / 40.0;
    std::size_t n = y0.size();
    double t = t0;
    State y = y0;
    double h = opts.initial_step;
    if (h <= 0.0) h = std::max(1e-8, (t1 - t0) * 1e-3);
    h = std::min(h, t1 - t0);
    State k1 = f(t, y);
    sol.t.push_back(t);
    sol.y.push_back(y);
    int steps = 0;
    while (t < t1 - 1e-14 && steps < static_cast<int>(opts.max_steps)) {
        ++steps;
        if (t + h > t1) h = t1 - t;
        h = std::min(h, opts.max_step);
        State k2 = f(t + c2 * h, detail::axpy(h * a21, k1, y));
        State k3 = f(t + c3 * h, detail::axpy(h * a31, k1, detail::axpy(h * a32, k2, y)));
        State k4 = f(t + c4 * h, detail::axpy(h * a41, k1, detail::axpy(h * a42, k2, detail::axpy(h * a43, k3, y))));
        State k5 = f(t + c5 * h, detail::axpy(h * a51, k1, detail::axpy(h * a52, k2, detail::axpy(h * a53, k3, detail::axpy(h * a54, k4, y)))));
        State k6 = f(t + h, detail::axpy(h * a61, k1, detail::axpy(h * a62, k2, detail::axpy(h * a63, k3, detail::axpy(h * a64, k4, detail::axpy(h * a65, k5, y))))));
        State y5 = detail::axpy(h * a71, k1, detail::axpy(h * a73, k3, detail::axpy(h * a74, k4, detail::axpy(h * a75, k5, detail::axpy(h * a76, k6, y)))));
        State k7 = f(t + h, y5);
        double scale = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            double e = (e1 * k1[i] + e3 * k3[i] + e4 * k4[i] + e5 * k5[i] + e6 * k6[i] + e7 * k7[i]) * h;
            double sc = opts.tolerance + opts.tolerance * std::max(std::fabs(y[i]), std::fabs(y5[i]));
            scale = std::max(scale, std::fabs(e) / sc);
        }

        if (scale <= 1.0 || h <= opts.min_step) {
            t += h;
            y = y5;
            sol.t.push_back(t);
            sol.y.push_back(y);
            k1 = k7;
            double factor = (scale < 1e-8) ? 5.0 : 0.9 * std::pow(scale, -0.2);
            factor = std::min(5.0, std::max(0.2, factor));
            h = std::min(h * factor, opts.max_step);
        } else {
            double factor = 0.9 * std::pow(scale, -0.25);
            factor = std::min(1.0, std::max(0.2, factor));
            h = std::max(h * factor, opts.min_step);
            k1 = f(t, y);
        }
    }
    return sol;
}

inline OdeSolution solve(const OdeFn& f, State y0, double t0, double t1, double dt) {
    return rk4(f, std::move(y0), t0, t1, dt);
}
}
}
