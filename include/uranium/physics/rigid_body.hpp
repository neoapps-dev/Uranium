#pragma once
#include "uranium/core/config.hpp"
#include "uranium/math/matrix.hpp"
#include "uranium/math/quaternion.hpp"
#include "uranium/math/vector.hpp"
#include "uranium/physics/collision.hpp"
#include <cmath>
namespace uranium {
namespace physics {
using Vec3 = Vector<double, 3>;
using Quat = Quaternion<double>;
using Mat3 = Matrix<double, 3, 3>;
struct RigidBody {
    Vec3 position{0.0, 0.0, 0.0};
    Quat orientation{1.0, 0.0, 0.0, 0.0};
    Vec3 linear_velocity{0.0, 0.0, 0.0};
    Vec3 angular_velocity{0.0, 0.0, 0.0};
    Vec3 force{0.0, 0.0, 0.0};
    Vec3 torque{0.0, 0.0, 0.0};
    double mass = 1.0;
    double inv_mass = 1.0;
    Mat3 inertia_local = Mat3::identity();
    Mat3 inv_inertia_local = Mat3::identity();
    double restitution = 0.3;
    double friction = 0.5;
    double linear_damping = 0.0;
    double angular_damping = 0.05;
    bool is_static = false;
    bool alive = true;
    Shape shape = SphereShape{0.5, Vec3{0.0, 0.0, 0.0}};
    RigidBody() = default;
    static RigidBody point_mass(double m, Vec3 pos = Vec3{0.0, 0.0, 0.0}) {
        RigidBody b;
        b.mass = m;
        b.inv_mass = m > 0.0 ? 1.0 / m : 0.0;
        b.position = pos;
        b.shape = SphereShape{0.0, Vec3{0.0, 0.0, 0.0}};
        if (m <= 0.0) b.make_static();
        return b;
    }

    static RigidBody solid_sphere(double radius, double density = 1000.0, Vec3 pos = Vec3{0.0, 0.0, 0.0}) {
        RigidBody b;
        b.mass = density * (4.0 / 3.0) * 3.14159265358979323846 * radius * radius * radius;
        b.inv_mass = 1.0 / b.mass;
        double i = 0.4 * b.mass * radius * radius;
        b.inertia_local = Mat3::scale(Vec3{i, i, i});
        b.inv_inertia_local = Mat3::scale(Vec3{1.0 / i, 1.0 / i, 1.0 / i});
        b.position = pos;
        b.shape = SphereShape{radius, Vec3{0.0, 0.0, 0.0}};
        return b;
    }

    static RigidBody solid_box(Vec3 half_extents, double density = 1000.0, Vec3 pos = Vec3{0.0, 0.0, 0.0}) {
        RigidBody b;
        Vec3 full = half_extents * 2.0;
        b.mass = density * full.x * full.y * full.z;
        b.inv_mass = 1.0 / b.mass;
        double ix = b.mass * (full.y * full.y + full.z * full.z) / 12.0;
        double iy = b.mass * (full.x * full.x + full.z * full.z) / 12.0;
        double iz = b.mass * (full.x * full.x + full.y * full.y) / 12.0;
        b.inertia_local = Mat3::scale(Vec3{ix, iy, iz});
        b.inv_inertia_local = Mat3::scale(Vec3{1.0 / ix, 1.0 / iy, 1.0 / iz});
        b.position = pos;
        b.shape = BoxShape{half_extents, Vec3{0.0, 0.0, 0.0}};
        return b;
    }

    static RigidBody hollow_sphere(double radius, double density = 1000.0, Vec3 pos = Vec3{0.0, 0.0, 0.0}) {
        RigidBody b;
        b.mass = density * 4.0 * 3.14159265358979323846 * radius * radius * radius;
        b.inv_mass = 1.0 / b.mass;
        double i = (2.0 / 3.0) * b.mass * radius * radius;
        b.inertia_local = Mat3::scale(Vec3{i, i, i});
        b.inv_inertia_local = Mat3::scale(Vec3{1.0 / i, 1.0 / i, 1.0 / i});
        b.position = pos;
        b.shape = SphereShape{radius, Vec3{0.0, 0.0, 0.0}};
        return b;
    }

    void make_static() {
        is_static = true;
        mass = 0.0;
        inv_mass = 0.0;
        inv_inertia_local = Mat3::zero();
        linear_velocity = Vec3{0.0, 0.0, 0.0};
        angular_velocity = Vec3{0.0, 0.0, 0.0};
    }

    void make_dynamic(double new_mass) {
        is_static = false;
        mass = new_mass;
        inv_mass = new_mass > 0.0 ? 1.0 / new_mass : 0.0;
    }

    URANIUM_NODISCARD Mat3 rotation_matrix() const { return orientation.to_matrix(); }
    URANIUM_NODISCARD Mat3 inv_inertia_world() const {
        if (is_static) return Mat3::zero();
        Mat3 r = orientation.to_matrix();
        return r * inv_inertia_local * r.transposed();
    }

    URANIUM_NODISCARD Vec3 world_point(const Vec3& local) const {
        return position + orientation.rotate(local);
    }

    URANIUM_NODISCARD Vec3 local_point(const Vec3& world) const {
        return orientation.conjugate().rotate(world - position);
    }

    URANIUM_NODISCARD Vec3 point_velocity(const Vec3& world_point) const {
        Vec3 r = world_point - position;
        return linear_velocity + cross(angular_velocity, r);
    }

    URANIUM_NODISCARD double kinetic_energy() const {
        if (is_static) return 0.0;
        Vec3 ang_body = orientation.conjugate().rotate(angular_velocity);
        Vec3 l = inertia_local * ang_body;
        double rot = dot(l, ang_body);
        return 0.5 * mass * linear_velocity.squared_norm() + 0.5 * rot;
    }

    URANIUM_NODISCARD Vec3 linear_momentum() const { return is_static ? Vec3{0.0, 0.0, 0.0} : linear_velocity * mass; }
    RigidBody& apply_force(Vec3 f) {
        if (!is_static) force += f;
        return *this;
    }

    RigidBody& apply_force_at(Vec3 f, Vec3 world_point) {
        if (is_static) return *this;
        force += f;
        Vec3 r = world_point - position;
        torque += cross(r, f);
        return *this;
    }

    RigidBody& apply_torque(Vec3 t) {
        if (!is_static) torque += t;
        return *this;
    }

    RigidBody& apply_impulse(Vec3 j, Vec3 world_point) {
        if (is_static) return *this;
        linear_velocity += j * inv_mass;
        Vec3 r = world_point - position;
        angular_velocity += inv_inertia_world() * cross(r, j);
        return *this;
    }

    RigidBody& apply_angular_impulse(Vec3 j) {
        if (!is_static) return *this;
        angular_velocity += inv_inertia_world() * j;
        return *this;
    }

    RigidBody& clear() {
        force = Vec3{0.0, 0.0, 0.0};
        torque = Vec3{0.0, 0.0, 0.0};
        return *this;
    }

    void integrate(double dt) {
        if (is_static || !alive) return;
        linear_velocity += force * (inv_mass * dt);
        angular_velocity += inv_inertia_world() * torque * dt;
        if (linear_damping > 0.0) linear_velocity *= std::exp(-linear_damping * dt);
        if (angular_damping > 0.0) angular_velocity *= std::exp(-angular_damping * dt);
        position += linear_velocity * dt;
        Quat omega{0.0, angular_velocity.x, angular_velocity.y, angular_velocity.z};
        Quat dq = omega * orientation;
        orientation = Quaternion<double>(orientation.w + dq.w * (0.5 * dt), orientation.x + dq.x * (0.5 * dt), orientation.y + dq.y * (0.5 * dt), orientation.z + dq.z * (0.5 * dt));
        orientation = orientation.normalized();
        clear();
    }
};
}
}
