#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>
namespace uranium {
namespace numerical {
class LinearInterpolator {
public:
    LinearInterpolator() = default;
    LinearInterpolator(std::vector<double> xs, std::vector<double> ys)
        : m_x(std::move(xs)), m_y(std::move(ys)) {
        sort_data();
    }

    void set_data(std::vector<double> xs, std::vector<double> ys) {
        m_x = std::move(xs);
        m_y = std::move(ys);
        sort_data();
    }

    std::size_t size() const { return m_x.size(); }
    const std::vector<double>& x() const { return m_x; }
    const std::vector<double>& y() const { return m_y; }
    double operator()(double x) const {
        if (m_x.empty()) return 0.0;
        if (m_x.size() == 1) return m_y[0];
        if (x <= m_x.front()) return m_y.front();
        if (x >= m_x.back()) return m_y.back();
        std::size_t i = upper_index(x);
        double x0 = m_x[i - 1], x1 = m_x[i];
        double y0 = m_y[i - 1], y1 = m_y[i];
        double t = (x - x0) / (x1 - x0);
        return y0 + (y1 - y0) * t;
    }

    double extrap_left(double x) const {
        if (m_x.size() < 2) return (*this)(x);
        double t = (x - m_x[0]) / (m_x[1] - m_x[0]);
        return m_y[0] + (m_y[1] - m_y[0]) * t;
    }

    double extrap_right(double x) const {
        if (m_x.size() < 2) return (*this)(x);
        std::size_t n = m_x.size();
        double t = (x - m_x[n - 2]) / (m_x[n - 1] - m_x[n - 2]);
        return m_y[n - 2] + (m_y[n - 1] - m_y[n - 2]) * t;
    }

private:
    void sort_data() {
        std::size_t n = std::min(m_x.size(), m_y.size());
        m_x.resize(n);
        m_y.resize(n);
        std::vector<std::size_t> order(n);
        for (std::size_t i = 0; i < n; ++i) order[i] = i;
        std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return m_x[a] < m_x[b]; });
        std::vector<double> xs(n), ys(n);
        for (std::size_t i = 0; i < n; ++i) {
            xs[i] = m_x[order[i]];
            ys[i] = m_y[order[i]];
        }
        m_x.swap(xs);
        m_y.swap(ys);
    }

    std::size_t upper_index(double x) const {
        std::size_t lo = 1, hi = m_x.size() - 1;
        while (lo < hi) {
            std::size_t mid = (lo + hi) / 2;
            if (m_x[mid] < x) lo = mid + 1;
            else hi = mid;
        }
        return lo;
    }

    std::vector<double> m_x;
    std::vector<double> m_y;
};

class CubicSpline {
public:
    CubicSpline() = default;
    CubicSpline(std::vector<double> xs, std::vector<double> ys) { set_data(std::move(xs), std::move(ys)); }
    void set_data(std::vector<double> xs, std::vector<double> ys) {
        std::size_t n = std::min(xs.size(), ys.size());
        xs.resize(n);
        ys.resize(n);
        std::vector<std::size_t> order(n);
        for (std::size_t i = 0; i < n; ++i) order[i] = i;
        std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return xs[a] < xs[b]; });
        m_x.resize(n);
        m_y.resize(n);
        for (std::size_t i = 0; i < n; ++i) {
            m_x[i] = xs[order[i]];
            m_y[i] = ys[order[i]];
        }
        if (m_x.size() < 3) {
            m_c.assign(m_x.size(), 0.0);
            m_b.assign(m_x.size() - 1, 0.0);
            m_d.assign(m_x.size() > 0 ? m_x.size() - 1 : 0, 0.0);
            m_a.resize(std::max<std::size_t>(m_x.size(), 1));
            for (std::size_t i = 0; i + 1 < m_x.size(); ++i) m_a[i] = m_y[i];
            if (!m_a.empty()) m_a.back() = m_y.back();
            return;
        }
        std::size_t n2 = m_x.size();
        std::vector<double> h(n2 - 1), alpha(n2);
        for (std::size_t i = 0; i + 1 < n2; ++i) h[i] = m_x[i + 1] - m_x[i];
        for (std::size_t i = 1; i + 1 < n2; ++i) {
            double denom = h[i - 1] + h[i];
            alpha[i] = (T3 * (m_y[i + 1] - m_y[i]) / h[i] - T3 * (m_y[i] - m_y[i - 1]) / h[i - 1]) / denom;
        }
        std::vector<double> l(n2), mu(n2), z(n2);
        l[0] = 1.0;
        mu[0] = 0.0;
        z[0] = 0.0;
        for (std::size_t i = 1; i + 1 < n2; ++i) {
            l[i] = 2.0 * (m_x[i + 1] - m_x[i - 1]) - h[i - 1] * mu[i - 1];
            mu[i] = h[i] / l[i];
            z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
        }
        l[n2 - 1] = 1.0;
        z[n2 - 1] = 0.0;
        m_c.assign(n2, 0.0);
        m_b.assign(n2 - 1, 0.0);
        m_a.assign(n2, 0.0);
        m_d.assign(n2 > 0 ? n2 - 1 : 0, 0.0);
        for (std::size_t j = n2 - 1; j-- > 0;) {
            m_c[j] = z[j] - mu[j] * m_c[j + 1];
            m_b[j] = ((m_y[j + 1] - m_y[j]) / h[j]) - h[j] * (m_c[j + 1] + T2 * m_c[j]) / T3;
            m_a[j] = m_y[j];
        }
        m_a[n2 - 1] = m_y[n2 - 1];
        for (std::size_t j = 0; j + 1 < n2; ++j) m_d[j] = (m_c[j + 1] - m_c[j]) / (T3 * h[j]);
    }

    std::size_t size() const { return m_x.size(); }
    const std::vector<double>& x() const { return m_x; }
    const std::vector<double>& y() const { return m_y; }
    double operator()(double x) const {
        if (m_x.empty()) return 0.0;
        if (m_x.size() == 1) return m_y[0];
        if (x <= m_x.front()) return m_y.front();
        if (x >= m_x.back()) return m_y.back();
        std::size_t i = 1;
        while (i + 1 < m_x.size() && m_x[i] < x) ++i;
        double dx = x - m_x[i - 1];
        return m_a[i - 1] + dx * (m_b[i - 1] + dx * (m_c[i - 1] + dx * m_d[i - 1]));
    }

    double derivative(double x) const {
        if (m_x.size() < 2) return 0.0;
        if (x <= m_x.front()) x = m_x.front() + 1e-12;
        if (x >= m_x.back()) x = m_x.back() - 1e-12;
        std::size_t i = 1;
        while (i + 1 < m_x.size() && m_x[i] < x) ++i;
        double dx = x - m_x[i - 1];
        return m_b[i - 1] + dx * (T2 * m_c[i - 1] + T3 * m_d[i - 1] * dx);
    }

private:
    static constexpr double T2 = 2.0;
    static constexpr double T3 = 3.0;
    std::vector<double> m_x, m_y, m_a, m_b, m_c, m_d;
};

template <class T>
T hermite(T p0, T p1, T m0, T m1, T t) {
    T t2 = t * t;
    T t3 = t2 * t;
    return (T(2) * t3 - T(3) * t2 + T(1)) * p0 + (t3 - T(2) * t2 + t) * m0 + (T(-2) * t3 + T(3) * t2) * p1 + (t3 - t2) * m1;
}

inline double sample_linear(const std::vector<double>& data, double u) {
    if (data.empty()) return 0.0;
    if (data.size() == 1) return data[0];
    double pos = u * static_cast<double>(data.size() - 1);
    if (pos <= 0.0) return data.front();
    if (pos >= static_cast<double>(data.size() - 1)) return data.back();
    std::size_t i = static_cast<std::size_t>(pos);
    double t = pos - static_cast<double>(i);
    return data[i] + (data[i + 1] - data[i]) * t;
}

inline std::vector<double> resample(const std::vector<double>& data, std::size_t count) {
    std::vector<double> out(count, 0.0);
    if (data.empty() || count == 0) return out;
    if (count == 1) {
        out[0] = data.front();
        return out;
    }
    for (std::size_t i = 0; i < count; ++i) {
        double u = static_cast<double>(i) / static_cast<double>(count - 1);
        out[i] = sample_linear(data, u);
    }
    return out;
}
}
}
