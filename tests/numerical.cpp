#include "check.hpp"
#include "uranium/math/numerical/numerical.hpp"
#include "uranium/math/stats/stats.hpp"
#include <cmath>
using namespace uranium;
using namespace uranium::numerical;
int main() {
    DynMatrix<double> A{{1, 1}, {1, 2}, {1, 3}, {1, 4}};
    auto ls = least_squares(A, std::vector<double>{6, 5, 7, 10});
    CHECK_NEAR(ls[0], 3.5, 1e-8);
    CHECK_NEAR(ls[1], 1.4, 1e-8);
    auto fit = polynomial_fit({1, 2, 3, 4, 5}, {1, 4, 9, 16, 25}, 2);
    CHECK_NEAR(fit.coefficients()[2], 1.0, 1e-9);
    CHECK_NEAR(fit.coefficients()[1], 0.0, 1e-9);
    CHECK_NEAR(fit.coefficients()[0], 0.0, 1e-9);
    DynMatrix<double> M{{12, -51, 4}, {6, 167, -68}, {-4, 24, -41}};
    auto qrd = qr(M);
    auto Q = qrd.explicit_q();
    auto QtQ = Q.transposed() * Q;
    CHECK_NEAR((QtQ - DynMatrix<double>::identity(3)).max_abs(), 0.0, 1e-10);
    CHECK_NEAR(((Q * qrd.r) - M).max_abs(), 0.0, 1e-10);
    CubicSpline sp({0, 1, 2, 3}, {0, 1, 4, 9});
    CHECK_NEAR(sp(0.0), 0.0, 1e-10);
    CHECK_NEAR(sp(1.0), 1.0, 1e-10);
    CHECK_NEAR(sp(3.0), 9.0, 1e-10);
    CHECK_NEAR(sp(1.5), 2.35, 1e-6);
    auto spectrum = fft(std::vector<double>{0, 1, 0, -1, 0, 1, 0, -1});
    CHECK_NEAR(spectrum[1].real(), 0.0, 1e-9);
    CHECK_NEAR(spectrum[1].imag(), 0.0, 1e-9);
    CHECK_NEAR(spectrum[2].real(), 0.0, 1e-9);
    CHECK_NEAR(spectrum[2].imag(), -4.0, 1e-9);
    auto sol = rk45([](double, const State& y) { return State{y[1], -y[0]}; }, State{1.0, 0.0}, 0.0,
                    10.0);
    CHECK_NEAR(sol.at(10.0)[0], std::cos(10.0), 1e-5);
    CHECK(sol.size() > 10);
    auto sol4 = rk4([](double, const State& y) { return State{y[1], -y[0]}; }, State{1.0, 0.0}, 0.0,
                    10.0, 0.001);
    CHECK_NEAR(sol4.at(10.0)[0], std::cos(10.0), 1e-5);
    double area = simpson([](double x) { return x * x; }, 0.0, 2.0, 100);
    CHECK_NEAR(area, 8.0 / 3.0, 1e-6);
    auto root = newton<double>([](double x) { return x * x - 2.0; },
                               [](double x) { return 2.0 * x; }, 1.0);
    CHECK_NEAR(root.value, std::sqrt(2.0), 1e-10);
    auto root2 = bisection<double>([](double x) { return x * x - 4.0; }, 0.0, 5.0);
    CHECK_NEAR(root2.value, 2.0, 1e-7);
    auto opt = gradient_descent(
        [](const std::vector<double>& v) { return (v[0] - 3) * (v[0] - 3) + (v[1] + 2) * (v[1] + 2); },
        [](const std::vector<double>& v) {
            return std::vector<double>{2 * (v[0] - 3), 2 * (v[1] + 2)};
        },
        std::vector<double>{0.0, 0.0});
    CHECK_NEAR(opt.x[0], 3.0, 1e-5);
    CHECK_NEAR(opt.x[1], -2.0, 1e-5);
    DynMatrix<double> S{{4, 1, 1}, {1, 3, -1}, {1, -1, 2}};
    auto eig = eigen_symmetric(S);
    CHECK(eig.converged);
    CHECK_NEAR(eig.eigenvalues[0] + eig.eigenvalues[1] + eig.eigenvalues[2], 9.0, 1e-6);
    std::vector<double> data{2, 4, 4, 4, 5, 5, 7, 9};
    CHECK_NEAR(stats::mean(data), 5.0, 1e-12);
    CHECK_NEAR(stats::median(data), 4.5, 1e-12);
    CHECK_NEAR(stats::stddev(data, 1), 2.13808993529939, 1e-9);
    auto fr = stats::fit_linear({1, 2, 3, 4, 5}, {2, 4, 6, 8, 10});
    CHECK_NEAR(fr.slope, 2.0, 1e-9);
    CHECK_NEAR(fr.intercept, 0.0, 1e-9);
    Polynomial<double> p({-6, 11, -6, 1});
    auto roots = p.real_roots();
    CHECK(roots.size() == 3);
    CHECK_NEAR(roots[0], 1.0, 1e-6);
    stats::Random rng(42);
    double s1 = rng.normal(0.0, 1.0);
    stats::Random rng2(42);
    double s2 = rng2.normal(0.0, 1.0);
    CHECK_NEAR(s1, s2, 1e-12);
    return 0;
}
