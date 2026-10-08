#pragma once
#include "uranium/core/error.hpp"
#include "uranium/math/functions.hpp"
#include <cmath>
#include <complex>
#include <cstddef>
#include <vector>
namespace uranium {
namespace numerical {
using Cplx = std::complex<double>;
inline bool is_power_of_two(std::size_t n) { return n > 0 && (n & (n - 1)) == 0; }
inline std::size_t next_power_of_two(std::size_t n) {
    std::size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

inline void fft_in_place(std::vector<Cplx>& a, bool inverse = false) {
    std::size_t n = a.size();
    if (n == 0) return;
    if (!is_power_of_two(n)) throw ValueError("fft size must be a power of two");
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    for (std::size_t len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * pi / static_cast<double>(len) * (inverse ? 1.0 : -1.0);
        Cplx wlen(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len) {
            Cplx w(1.0, 0.0);
            for (std::size_t j = 0; j < len / 2; ++j) {
                Cplx u = a[i + j];
                Cplx v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (inverse) {
        for (auto& x : a) x /= static_cast<double>(n);
    }
}

inline std::vector<Cplx> fft(const std::vector<double>& signal, bool pad = true) {
    std::size_t n = signal.size();
    std::size_t size = pad ? next_power_of_two(n) : n;
    if (!pad && !is_power_of_two(n)) throw ValueError("fft size must be a power of two");
    std::vector<Cplx> a(size, Cplx(0.0, 0.0));
    for (std::size_t i = 0; i < n; ++i) a[i] = Cplx(signal[i], 0.0);
    fft_in_place(a, false);
    return a;
}

inline std::vector<Cplx> ifft(const std::vector<Cplx>& spectrum) {
    std::vector<Cplx> a = spectrum;
    fft_in_place(a, true);
    return a;
}

inline std::vector<double> real_signal(const std::vector<Cplx>& a) {
    std::vector<double> out(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) out[i] = a[i].real();
    return out;
}

inline std::vector<double> magnitude_spectrum(const std::vector<Cplx>& spectrum, bool one_sided = true) {
    std::size_t n = spectrum.size();
    std::size_t count = one_sided ? n / 2 + 1 : n;
    std::vector<double> out(count);
    for (std::size_t i = 0; i < count; ++i) out[i] = std::abs(spectrum[i]);
    return out;
}

inline std::vector<double> power_spectrum(const std::vector<Cplx>& spectrum, bool one_sided = true) {
    std::size_t n = spectrum.size();
    std::size_t count = one_sided ? n / 2 + 1 : n;
    std::vector<double> out(count);
    for (std::size_t i = 0; i < count; ++i) out[i] = std::norm(spectrum[i]);
    return out;
}

inline std::vector<double> phase_spectrum(const std::vector<Cplx>& spectrum, bool one_sided = true) {
    std::size_t n = spectrum.size();
    std::size_t count = one_sided ? n / 2 + 1 : n;
    std::vector<double> out(count);
    for (std::size_t i = 0; i < count; ++i) out[i] = std::arg(spectrum[i]);
    return out;
}

inline std::vector<double> frequencies(std::size_t fft_size, double sample_rate) {
    std::size_t count = fft_size / 2 + 1;
    std::vector<double> out(count);
    for (std::size_t i = 0; i < count; ++i) out[i] = static_cast<double>(i) * sample_rate / static_cast<double>(fft_size);
    return out;
}

inline double dominant_frequency(const std::vector<double>& magnitudes, double sample_rate, std::size_t fft_size) {
    if (magnitudes.size() < 2) return 0.0;
    std::size_t best = 1;
    for (std::size_t i = 1; i + 1 < magnitudes.size(); ++i) if (magnitudes[i] > magnitudes[best]) best = i;
    double df = sample_rate / static_cast<double>(fft_size);
    if (best + 1 < magnitudes.size()) {
        double y0 = magnitudes[best - 1];
        double y1 = magnitudes[best];
        double y2 = magnitudes[best + 1];
        double denom = y0 - 2.0 * y1 + y2;
        if (std::fabs(denom) > 1e-30) {
            double shift = 0.5 * (y0 - y2) / denom;
            if (std::fabs(shift) <= 1.0) return (static_cast<double>(best) + shift) * df;
        }
    }
    return static_cast<double>(best) * df;
}

enum class Window { None, Hann, Hamming, Blackman, Rectangle };
inline std::vector<double> make_window(Window kind, std::size_t n) {
    std::vector<double> w(n, 1.0);
    if (n < 2 || kind == Window::None || kind == Window::Rectangle) return w;
    for (std::size_t i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / static_cast<double>(n - 1);
        switch (kind) {
            case Window::Hann: w[i] = 0.5 - 0.5 * std::cos(2.0 * pi * t); break;
            case Window::Hamming: w[i] = 0.54 - 0.46 * std::cos(2.0 * pi * t); break;
            case Window::Blackman:
                w[i] = 0.42 - 0.5 * std::cos(2.0 * pi * t) + 0.08 * std::cos(4.0 * pi * t);
                break;
            default: break;
        }
    }
    return w;
}

inline std::vector<double> apply_window(const std::vector<double>& data, Window kind) {
    std::vector<double> w = make_window(kind, data.size());
    std::vector<double> out(data.size());
    for (std::size_t i = 0; i < data.size(); ++i) out[i] = data[i] * w[i];
    return out;
}
}
}
