#include "check.hpp"
#include "uranium/math/math.hpp"
using namespace uranium;
int main() {
    Vec3 a{1, 2, 3};
    Vec3 b{4, 5, 6};
    CHECK_NEAR(dot(a, b), 32.0f, 1e-6f);
    Vec3 c = cross(a, b);
    CHECK_NEAR(c.x, -3.0f, 1e-6f);
    CHECK_NEAR(c.y, 6.0f, 1e-6f);
    CHECK_NEAR(c.z, -3.0f, 1e-6f);
    CHECK_NEAR(length(Vec3{3, 4, 0}), 5.0f, 1e-6f);
    CHECK_NEAR(normalize(Vec3{10, 0, 0}).x, 1.0f, 1e-6f);
    CHECK_NEAR(a.norm(), std::sqrt(14.0f), 1e-5f);
    Mat4 m = Mat4::rotation_z(0.5f) * Mat4::translation(Vec3{1, 2, 3});
    Mat4 ident = m * m.inverted();
    CHECK_NEAR(ident.m[0][0], 1.0f, 1e-4f);
    CHECK_NEAR(ident.m[0][1], 0.0f, 1e-4f);
    CHECK_NEAR(ident.m[3][2], 0.0f, 1e-4f);
    Mat3 r = Mat3::rotation_y(0.3f);
    CHECK_NEAR(r.determinant(), 1.0f, 1e-5f);
    CHECK_NEAR((r * r.transposed() - Mat3::identity()).max_abs(), 0.0f, 1e-5f);
    Quat q = Quat::from_axis_angle(Vec3{0, 1, 0}, 0.7f);
    Vec3 v = q.rotate(Vec3{1, 0, 0});
    CHECK_NEAR(v.x, std::cos(0.7f), 1e-5f);
    CHECK_NEAR(v.z, -std::sin(0.7f), 1e-5f);
    Quat qn = q.normalized();
    CHECK_NEAR(qn.w * qn.w + qn.x * qn.x + qn.y * qn.y + qn.z * qn.z, 1.0f, 1e-5f);
    Complex<double> z(0.0, 3.141592653589793);
    auto w = exp(z);
    CHECK_NEAR(w.re, -1.0, 1e-6);
    CHECK_NEAR(w.im, 0.0, 1e-6);
    Mat2 m2{{0.0f, -1.0f}, {1.0f, 0.0f}};
    Vec2 r2 = m2 * Vec2{1.0f, 0.0f};
    CHECK_NEAR(r2.x, 0.0f, 1e-6f);
    CHECK_NEAR(r2.y, 1.0f, 1e-6f);
    CHECK_NEAR(m2.determinant(), 1.0f, 1e-6f);
    return 0;
}
