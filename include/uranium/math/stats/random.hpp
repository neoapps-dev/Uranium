#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>
namespace uranium {
namespace stats {
class Random {
public:
    Random() : m_rng(std::random_device{}()) {}
    explicit Random(std::uint64_t seed) : m_rng(seed) { m_has_spare = false; }
    void seed(std::uint64_t s) {
        m_rng.seed(s);
        m_has_spare = false;
    }

    std::mt19937_64& engine() { return m_rng; }
    double uniform(double lo = 0.0, double hi = 1.0) {
        std::uniform_real_distribution<double> d(lo, hi);
        return d(m_rng);
    }

    int uniform_int(int lo, int hi) {
        std::uniform_int_distribution<int> d(lo, hi);
        return d(m_rng);
    }

    double normal(double mean = 0.0, double sd = 1.0) {
        if (m_has_spare) {
            m_has_spare = false;
            return mean + sd * m_spare;
        }
        double u1 = uniform();
        double u2 = uniform();
        if (u1 < 1e-300) u1 = 1e-300;
        double r = std::sqrt(-2.0 * std::log(u1));
        double theta = 6.28318530717958647692 * u2;
        m_spare = r * std::sin(theta);
        m_has_spare = true;
        return mean + sd * (r * std::cos(theta));
    }

    double exponential(double lambda = 1.0) {
        double u = uniform();
        if (u < 1e-300) u = 1e-300;
        return -std::log(u) / lambda;
    }

    double cauchy(double location = 0.0, double scale = 1.0) {
        double u = uniform() - 0.5;
        if (std::fabs(u) < 1e-300) u = 1e-300;
        return location + scale * std::tan(3.14159265358979323846 * u);
    }

    double lognormal(double mu = 0.0, double sigma = 1.0) { return std::exp(normal(mu, sigma)); }
    double gamma(double shape, double scale = 1.0) {
        if (shape < 1.0) {
            double u = uniform();
            if (u < 1e-300) u = 1e-300;
            return gamma(shape + 1.0, scale) * std::pow(u, 1.0 / shape);
        }
        double d = shape - 1.0 / 3.0;
        double c = 1.0 / std::sqrt(9.0 * d);
        for (;;) {
            double x = normal();
            double v = 1.0 + c * x;
            if (v <= 0.0) continue;
            v = v * v * v;
            double u = uniform();
            if (u < 1.0 - 0.0331 * x * x * x * x) return d * v * scale;
            if (std::log(u) < 0.5 * x * x + d * (1.0 - v + std::log(v))) return d * v * scale;
        }
    }

    double beta(double a, double b) {
        double x = gamma(a, 1.0);
        double y = gamma(b, 1.0);
        double s = x + y;
        return s > 0.0 ? x / s : 0.0;
    }

    double triangular(double lo = 0.0, double hi = 1.0, double mode = 0.5) {
        double u = uniform();
        double total = hi - lo;
        double fc = (mode - lo) / total;
        if (u < fc) return lo + std::sqrt(u * total * (mode - lo));
        return hi - std::sqrt((1.0 - u) * total * (hi - mode));
    }

    bool bernoulli(double p = 0.5) { return uniform() < p; }
    double poisson(double lambda) {
        if (lambda <= 0.0) return 0.0;
        if (lambda < 30.0) {
            double l = std::exp(-lambda);
            std::size_t k = 0;
            double p = 1.0;
            do {
                ++k;
                p *= uniform();
            } while (p > l);
            return static_cast<double>(k - 1);
        }
        double mean = std::floor(lambda);
        double sigma = std::sqrt(lambda);
        double x = normal();
        double y = x * sigma + mean;
        return y < 0.0 ? 0.0 : std::floor(y + 0.5);
    }

    std::vector<double> sample_normal(std::size_t count, double mean = 0.0, double sd = 1.0) {
        std::vector<double> out(count);
        for (std::size_t i = 0; i < count; ++i) out[i] = normal(mean, sd);
        return out;
    }

    std::vector<double> sample_uniform(std::size_t count, double lo = 0.0, double hi = 1.0) {
        std::vector<double> out(count);
        for (std::size_t i = 0; i < count; ++i) out[i] = uniform(lo, hi);
        return out;
    }

    template <class It>
    void shuffle(It first, It last) {
        std::shuffle(first, last, m_rng);
    }

    template <class Container>
    void shuffle(Container& c) {
        std::shuffle(c.begin(), c.end(), m_rng);
    }

    template <class Container>
    const auto& choice(const Container& c) {
        return c[static_cast<std::size_t>(uniform_int(0, static_cast<int>(c.size()) - 1))];
    }

    static Random& global() {
        static Random instance(std::random_device{}());
        return instance;
    }

private:
    std::mt19937_64 m_rng;
    double m_spare = 0.0;
    bool m_has_spare = false;
};

inline double rand_uniform(double lo = 0.0, double hi = 1.0) { return Random::global().uniform(lo, hi); }
inline double rand_normal(double mean = 0.0, double sd = 1.0) { return Random::global().normal(mean, sd); }
inline int rand_int(int lo, int hi) { return Random::global().uniform_int(lo, hi); }
}
}
