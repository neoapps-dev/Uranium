#pragma once
#include "uranium/math/vector.hpp"
#include <cmath>
#include <functional>
namespace uranium {
namespace physics {
using Vec3 = Vector<double, 3>;
using ForceField = std::function<Vec3(const Vec3& position, const Vec3& velocity, double time)>;
inline ForceField operator+(ForceField a, ForceField b) {
    return [a = std::move(a), b = std::move(b)](const Vec3& p, const Vec3& v, double t) -> Vec3 {
        return a(p, v, t) + b(p, v, t);
    };
}

inline ForceField constant_force(Vec3 f) {
    return [f](const Vec3&, const Vec3&, double) -> Vec3 { return f; };
}

inline ForceField gravity(Vec3 g = Vec3{0.0, -9.81, 0.0}) {
    return constant_force(g);
}

inline ForceField harmonic(double k, Vec3 center = Vec3{0.0, 0.0, 0.0}) {
    return [k, center](const Vec3& p, const Vec3&, double) -> Vec3 { return (center - p) * k; };
}

inline ForceField linear_drag(double coefficient) {
    return [coefficient](const Vec3&, const Vec3& v, double) -> Vec3 { return v * -coefficient; };
}

inline ForceField quadratic_drag(double coefficient) {
    return [coefficient](const Vec3&, const Vec3& v, double) -> Vec3 {
        double s = v.norm();
        return v * (-coefficient * s);
    };
}

inline ForceField spring(Vec3 anchor, double stiffness, double rest_length = 0.0) {
    return [anchor, stiffness, rest_length](const Vec3& p, const Vec3&, double) -> Vec3 {
        Vec3 d = anchor - p;
        double len = d.norm();
        if (len < 1e-12) return Vec3{0.0, 0.0, 0.0};
        return d * (stiffness * (len - rest_length) / len);
    };
}

inline ForceField coulomb(Vec3 source, double strength) {
    return [source, strength](const Vec3& p, const Vec3&, double) -> Vec3 {
        Vec3 d = source - p;
        double r2 = d.squared_norm();
        if (r2 < 1e-12) return Vec3{0.0, 0.0, 0.0};
        return d * (strength / r2);
    };
}

inline ForceField lorentz(Vec3 magnetic_field, double charge) {
    return [magnetic_field, charge](const Vec3&, const Vec3& v, double) -> Vec3 {
        return cross(v, magnetic_field) * charge;
    };
}

inline ForceField damping_at(double linear, double quadratic) {
    return linear_drag(linear) + quadratic_drag(quadratic);
}

inline ForceField velocity_dependent(std::function<Vec3(const Vec3&)> f) {
    return [f = std::move(f)](const Vec3&, const Vec3& v, double) -> Vec3 { return f(v); };
}

inline ForceField position_dependent(std::function<Vec3(const Vec3&)> f) {
    return [f = std::move(f)](const Vec3& p, const Vec3&, double) -> Vec3 { return f(p); };
}

inline ForceField time_dependent(std::function<Vec3(double)> f) {
    return [f = std::move(f)](const Vec3&, const Vec3&, double t) -> Vec3 { return f(t); };
}
}
}
