#pragma once
#include "uranium/core/error.hpp"
#include "uranium/math/functions.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <vector>
namespace uranium {
namespace stats {
template <class It>
double sum(It first, It last) {
    double s = 0.0;
    for (; first != last; ++first) s += static_cast<double>(*first);
    return s;
}

inline double sum(const std::vector<double>& v) { return sum(v.begin(), v.end()); }
template <class It>
double mean(It first, It last) {
    std::size_t n = static_cast<std::size_t>(std::distance(first, last));
    return n == 0 ? 0.0 : sum(first, last) / static_cast<double>(n);
}

inline double mean(const std::vector<double>& v) { return mean(v.begin(), v.end()); }
template <class It>
double variance(It first, It last, int ddof = 0) {
    std::size_t n = static_cast<std::size_t>(std::distance(first, last));
    if (n <= static_cast<std::size_t>(ddof)) return 0.0;
    double m = mean(first, last);
    double acc = 0.0;
    for (; first != last; ++first) {
        double d = static_cast<double>(*first) - m;
        acc += d * d;
    }
    return acc / static_cast<double>(n - static_cast<std::size_t>(ddof));
}

inline double variance(const std::vector<double>& v, int ddof = 0) {
    return variance(v.begin(), v.end(), ddof);
}

template <class It>
double stddev(It first, It last, int ddof = 0) {
    return std::sqrt(variance(first, last, ddof));
}

inline double stddev(const std::vector<double>& v, int ddof = 0) {
    return stddev(v.begin(), v.end(), ddof);
}

inline double sem(const std::vector<double>& v, int ddof = 1) {
    if (v.empty()) return 0.0;
    double denom = static_cast<double>(v.size() - static_cast<std::size_t>(ddof));
    if (denom <= 0.0) return 0.0;
    return stddev(v, ddof) / std::sqrt(denom);
}

inline double median(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    std::vector<double> s = v;
    std::size_t n = s.size();
    std::nth_element(s.begin(), s.begin() + static_cast<std::ptrdiff_t>(n / 2), s.end());
    double mid = s[n / 2];
    if (n % 2 == 1) return mid;
    double lower = *std::max_element(s.begin(), s.begin() + static_cast<std::ptrdiff_t>(n / 2));
    return (lower + mid) * 0.5;
}

inline double mode(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    std::vector<double> s = v;
    std::sort(s.begin(), s.end());
    double best = s[0];
    std::size_t best_count = 1;
    std::size_t count = 1;
    for (std::size_t i = 1; i < s.size(); ++i) {
        if (s[i] == s[i - 1]) {
            ++count;
        } else {
            count = 1;
        }
        if (count > best_count) {
            best_count = count;
            best = s[i];
        }
    }
    return best;
}

inline double min_value(const std::vector<double>& v) {
    return v.empty() ? 0.0 : *std::min_element(v.begin(), v.end());
}

inline double max_value(const std::vector<double>& v) {
    return v.empty() ? 0.0 : *std::max_element(v.begin(), v.end());
}

inline double range_value(const std::vector<double>& v) {
    return v.empty() ? 0.0 : max_value(v) - min_value(v);
}

inline double quantile(const std::vector<double>& v, double q) {
    if (v.empty()) return 0.0;
    if (q <= 0.0) return min_value(v);
    if (q >= 1.0) return max_value(v);
    std::vector<double> s = v;
    std::sort(s.begin(), s.end());
    double pos = q * static_cast<double>(s.size() - 1);
    std::size_t i = static_cast<std::size_t>(pos);
    double frac = pos - static_cast<double>(i);
    if (i + 1 >= s.size()) return s.back();
    return s[i] + (s[i + 1] - s[i]) * frac;
}

inline double percentile(const std::vector<double>& v, double p) { return quantile(v, p / 100.0); }
inline double covariance(const std::vector<double>& a, const std::vector<double>& b, int ddof = 1) {
    std::size_t n = std::min(a.size(), b.size());
    if (n <= static_cast<std::size_t>(ddof)) return 0.0;
    double ma = 0.0, mb = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        ma += a[i];
        mb += b[i];
    }
    ma /= static_cast<double>(n);
    mb /= static_cast<double>(n);
    double acc = 0.0;
    for (std::size_t i = 0; i < n; ++i) acc += (a[i] - ma) * (b[i] - mb);
    return acc / static_cast<double>(n - static_cast<std::size_t>(ddof));
}

inline double correlation(const std::vector<double>& a, const std::vector<double>& b) {
    double sa = stddev(a, 1), sb = stddev(b, 1);
    if (sa <= 0.0 || sb <= 0.0) return 0.0;
    return covariance(a, b, 1) / (sa * sb);
}

inline std::vector<double> ranks(const std::vector<double>& v) {
    std::vector<std::size_t> order(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](std::size_t x, std::size_t y) { return v[x] < v[y]; });
    std::vector<double> out(v.size(), 0.0);
    std::size_t i = 0;
    while (i < order.size()) {
        std::size_t j = i;
        while (j + 1 < order.size() && v[order[j + 1]] == v[order[i]]) ++j;
        double r = 0.5 * (static_cast<double>(i) + static_cast<double>(j)) + 1.0;
        for (std::size_t k = i; k <= j; ++k) out[order[k]] = r;
        i = j + 1;
    }
    return out;
}

inline double spearman(const std::vector<double>& a, const std::vector<double>& b) {
    return correlation(ranks(a), ranks(b));
}

inline double skewness(const std::vector<double>& v) {
    std::size_t n = v.size();
    if (n < 3) return 0.0;
    double m = mean(v);
    double s = stddev(v, 0);
    if (s <= 0.0) return 0.0;
    double acc = 0.0;
    for (double x : v) acc += std::pow((x - m) / s, 3.0);
    return acc * static_cast<double>(n) / static_cast<double>((n - 1) * (n - 2));
}

inline double kurtosis(const std::vector<double>& v) {
    std::size_t n = v.size();
    if (n < 4) return 0.0;
    double m = mean(v);
    double s = stddev(v, 0);
    if (s <= 0.0) return 0.0;
    double acc = 0.0;
    for (double x : v) acc += std::pow((x - m) / s, 4.0);
    double num = static_cast<double>(n * (n + 1)) / static_cast<double>((n - 1) * (n - 2) * (n - 3));
    double term = 3.0 * static_cast<double>((n - 1) * (n - 1)) / static_cast<double>((n - 2) * (n - 3));
    return num * acc - term;
}

inline std::vector<double> zscores(const std::vector<double>& v) {
    double m = mean(v);
    double s = stddev(v, 0);
    std::vector<double> out(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) out[i] = s > 0.0 ? (v[i] - m) / s : 0.0;
    return out;
}

inline std::vector<double> normalize_minmax(const std::vector<double>& v, double lo = 0.0, double hi = 1.0) {
    double a = min_value(v), b = max_value(v);
    std::vector<double> out(v.size());
    double span = b - a;
    for (std::size_t i = 0; i < v.size(); ++i) out[i] = span > 0.0 ? lo + (hi - lo) * (v[i] - a) / span : lo;
    return out;
}

struct Histogram {
    std::vector<double> edges;
    std::vector<double> centers;
    std::vector<std::size_t> counts;
    std::size_t max_count = 0;
    std::size_t total = 0;
    std::size_t bins() const { return counts.size(); }
};

inline Histogram histogram(const std::vector<double>& data, std::size_t bins = 10) {
    Histogram h;
    if (data.empty() || bins == 0) return h;
    double lo = min_value(data), hi = max_value(data);
    if (hi <= lo) {
        hi = lo + 1.0;
    }
    h.edges.resize(bins + 1);
    h.counts.assign(bins, 0);
    double width = (hi - lo) / static_cast<double>(bins);
    for (std::size_t i = 0; i <= bins; ++i) h.edges[i] = lo + width * static_cast<double>(i);
    for (double x : data) {
        if (x < lo || x > hi) continue;
        std::size_t idx = static_cast<std::size_t>((x - lo) / width);
        if (idx >= bins) idx = bins - 1;
        ++h.counts[idx];
    }
    h.centers.resize(bins);
    for (std::size_t i = 0; i < bins; ++i) {
        h.centers[i] = (h.edges[i] + h.edges[i + 1]) * 0.5;
        h.total += h.counts[i];
        h.max_count = std::max(h.max_count, h.counts[i]);
    }
    return h;
}

inline Histogram histogram(const std::vector<double>& data, double lo, double hi, std::size_t bins) {
    Histogram h;
    if (data.empty() || bins == 0 || hi <= lo) return h;
    h.edges.resize(bins + 1);
    h.counts.assign(bins, 0);
    double width = (hi - lo) / static_cast<double>(bins);
    for (std::size_t i = 0; i <= bins; ++i) h.edges[i] = lo + width * static_cast<double>(i);
    for (double x : data) {
        if (x < lo || x >= hi) continue;
        std::size_t idx = static_cast<std::size_t>((x - lo) / width);
        if (idx >= bins) idx = bins - 1;
        ++h.counts[idx];
    }
    h.centers.resize(bins);
    for (std::size_t i = 0; i < bins; ++i) {
        h.centers[i] = (h.edges[i] + h.edges[i + 1]) * 0.5;
        h.total += h.counts[i];
        h.max_count = std::max(h.max_count, h.counts[i]);
    }
    return h;
}

struct Summary {
    std::size_t count = 0;
    double mean = 0.0;
    double stddev = 0.0;
    double min = 0.0;
    double q1 = 0.0;
    double median = 0.0;
    double q3 = 0.0;
    double max = 0.0;
};

inline Summary describe(const std::vector<double>& v) {
    Summary s;
    s.count = v.size();
    if (v.empty()) return s;
    s.mean = stats::mean(v);
    s.stddev = stats::stddev(v, 1);
    s.min = min_value(v);
    s.q1 = quantile(v, 0.25);
    s.median = stats::median(v);
    s.q3 = quantile(v, 0.75);
    s.max = max_value(v);
    return s;
}

struct LinearRegression {
    double slope = 0.0;
    double intercept = 0.0;
    double r = 0.0;
    double r2 = 0.0;
    double standard_error = 0.0;
    double operator()(double x) const { return intercept + slope * x; }
};

inline LinearRegression fit_linear(const std::vector<double>& xs, const std::vector<double>& ys) {
    LinearRegression out;
    std::size_t n = std::min(xs.size(), ys.size());
    if (n == 0) return out;
    if (n == 1) {
        out.intercept = ys[0];
        return out;
    }
    double mx = 0.0, my = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        mx += xs[i];
        my += ys[i];
    }
    mx /= static_cast<double>(n);
    my /= static_cast<double>(n);
    double sxy = 0.0, sxx = 0.0, syy = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        double dx = xs[i] - mx;
        double dy = ys[i] - my;
        sxy += dx * dy;
        sxx += dx * dx;
        syy += dy * dy;
    }
    out.slope = sxx > 0.0 ? sxy / sxx : 0.0;
    out.intercept = my - out.slope * mx;
    out.r = (sxx > 0.0 && syy > 0.0) ? sxy / std::sqrt(sxx * syy) : 0.0;
    out.r2 = out.r * out.r;
    if (n > 2 && sxx > 0.0) {
        double sse = 0.0;
        for (std::size_t i = 0; i < n; ++i) {
            double e = ys[i] - (out.intercept + out.slope * xs[i]);
            sse += e * e;
        }
        out.standard_error = std::sqrt(sse / static_cast<double>(n - 2) / sxx);
    }
    return out;
}
}
}
