#include "uranium/math/math.hpp"
#include "uranium/math/numerical/numerical.hpp"
#include "uranium/math/stats/stats.hpp"
#include <cstdio>
using namespace uranium;
using namespace uranium::numerical;
int main() {
    Vec3 a{1, 2, 3};
    Vec3 b{4, 5, 6};
    std::printf("cross(a,b) = (%g, %g, %g)  dot = %g  |a| = %g\n", cross(a, b).x, cross(a, b).y, cross(a, b).z, dot(a, b), length(a));
    Mat4 m = Mat4::rotation_z(0.5f) * Mat4::translation(Vec3{1, 2, 3});
    Mat4 ident = m * m.inverted();
    std::printf("M * M^-1 = I? m00=%.6f m01=%.6f\n", ident.m[0][0], ident.m[0][1]);
    Quat q = Quat::from_axis_angle(Vec3{0, 1, 0}, 0.7f);
    Vec3 v = q.rotate(Vec3{1, 0, 0});
    std::printf("quat rotate = (%.4f, %.4f, %.4f)\n", v.x, v.y, v.z);
    auto fit = stats::fit_linear({1, 2, 3, 4, 5}, {2.1, 3.9, 6.2, 7.8, 10.1});
    std::printf("linear fit  y = %.3f x + %.3f\n", fit.slope, fit.intercept);
    auto poly = polynomial_fit({1, 2, 3, 4, 5}, {1, 4, 9, 16, 25}, 2);
    std::printf("quadratic fit  c = [%.3f, %.3f, %.3f]\n", poly.coefficients()[0], poly.coefficients()[1], poly.coefficients()[2]);
    DynMatrix<double> A{{1, 1}, {1, 2}, {1, 3}, {1, 4}};
    auto ls = least_squares(A, std::vector<double>{6, 5, 7, 10});
    std::printf("least squares  intercept = %.4f  slope = %.4f\n", ls[0], ls[1]);
    DynMatrix<double> M{{12, -51, 4}, {6, 167, -68}, {-4, 24, -41}};
    auto qrd = qr(M);
    auto Q = qrd.explicit_q();
    auto QtQ = Q.transposed() * Q;
    std::printf("QR: |Q^T Q - I| = %.2e  |QR - M| = %.2e\n", (QtQ - DynMatrix<double>::identity(3)).max_abs(), ((Q * qrd.r) - M).max_abs());
    CubicSpline sp({0, 1, 2, 3}, {0, 1, 4, 9});
    std::printf("spline  s(1.5) = %.6f  (natural cubic)\n", sp(1.5));
    auto spectrum = fft(std::vector<double>{0, 1, 0, -1, 0, 1, 0, -1});
    std::printf("fft  X[1] = %.3f%+.3fi  X[2] = %.3f%+.3fi\n", spectrum[1].real(), spectrum[1].imag(), spectrum[2].real(), spectrum[2].imag());
    auto sol = rk45([](double, const State& y) { return State{y[1], -y[0]}; }, State{1.0, 0.0}, 0.0, 10.0);
    std::printf("rk45  y(10) = %.6f  (cos(10) = %.6f, %zu steps)\n", sol.at(10.0)[0], -0.8390715290764524, sol.size());
    double area = simpson([](double x) { return std::sin(x); }, 0.0, 3.141592653589793, 200);
    std::printf("simpson  int_0^pi sin = %.10f\n", area);
    auto root = newton<double>([](double x) { return x * x - 2.0; }, [](double x) { return 2.0 * x; }, 1.0);
    std::printf("newton  sqrt(2) = %.12f\n", root.value);
    std::vector<double> data{2, 4, 4, 4, 5, 5, 7, 9};
    std::printf("stats  mean = %.3f  median = %.3f  stddev = %.3f\n", stats::mean(data), stats::median(data), stats::stddev(data, 1));
    DynMatrix<double> S{{4, 1, 1}, {1, 3, -1}, {1, -1, 2}};
    auto eig = eigen_symmetric(S);
    std::printf("eigenvalues  %.4f  %.4f  %.4f\n", eig.eigenvalues[0], eig.eigenvalues[1], eig.eigenvalues[2]);
    return 0;
}
