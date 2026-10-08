#pragma once
#include "uranium/core/config.hpp"
#include "uranium/math/vector.hpp"
#include "uranium/physics/forces.hpp"
#include <cmath>
#include <vector>
namespace uranium {
namespace physics {
enum class Method {
    Euler,
    SemiImplicitEuler,
    VelocityVerlet,
    RK4
};

struct Particle {
    Vec3 position{0.0, 0.0, 0.0};
    Vec3 velocity{0.0, 0.0, 0.0};
    Vec3 acceleration{0.0, 0.0, 0.0};
    Vec3 force{0.0, 0.0, 0.0};
    double mass = 1.0;
    double damping = 0.0;
    bool alive = true;
    Particle() = default;
    explicit Particle(double m) : mass(m) {}
    Particle(double m, Vec3 pos) : position(pos), mass(m) {}
    Particle(double m, Vec3 pos, Vec3 vel) : position(pos), velocity(vel), mass(m) {}
    static Particle at(Vec3 position, double mass = 1.0) { return Particle(mass, position); }
    static Particle moving(Vec3 position, Vec3 velocity, double mass = 1.0) { return Particle(mass, position, velocity); }
    URANIUM_NODISCARD double inv_mass() const { return mass > 0.0 ? 1.0 / mass : 0.0; }
    URANIUM_NODISCARD double speed() const { return velocity.norm(); }
    URANIUM_NODISCARD double kinetic_energy() const { return 0.5 * mass * velocity.squared_norm(); }
    URANIUM_NODISCARD Vec3 momentum() const { return velocity * mass; }
    Particle& set_mass(double m) {
        mass = m;
        return *this;
    }
    Particle& set_velocity(Vec3 v) {
        velocity = v;
        return *this;
    }
    Particle& set_position(Vec3 p) {
        position = p;
        return *this;
    }
    Particle& set_damping(double d) {
        damping = d;
        return *this;
    }
    Particle& apply(Vec3 f) {
        force += f;
        return *this;
    }
    Particle& clear_force() {
        force = Vec3{0.0, 0.0, 0.0};
        return *this;
    }

    void integrate(double dt, const Vec3& net_force) {
        if (!alive || mass <= 0.0) return;
        acceleration = net_force * inv_mass();
        velocity += acceleration * dt;
        if (damping > 0.0) velocity *= std::exp(-damping * dt);
        position += velocity * dt;
        force = Vec3{0.0, 0.0, 0.0};
    }
};

namespace detail {
inline Vec3 evaluate(const std::vector<ForceField>& fields, const Vec3& p, const Vec3& v, double t) {
    Vec3 total{0.0, 0.0, 0.0};
    for (const auto& f : fields) total += f(p, v, t);
    return total;
}
}

inline void integrate(Particle& p, double dt, const std::vector<ForceField>& fields, double time = 0.0, Method method = Method::SemiImplicitEuler) {
    if (!p.alive || p.mass <= 0.0) return;
    Vec3 external = p.force + detail::evaluate(fields, p.position, p.velocity, time);
    switch (method) {
        case Method::Euler: {
            p.acceleration = external * p.inv_mass();
            p.position += p.velocity * dt;
            p.velocity += p.acceleration * dt;
            break;
        }
        case Method::SemiImplicitEuler: {
            p.acceleration = external * p.inv_mass();
            p.velocity += p.acceleration * dt;
            if (p.damping > 0.0) p.velocity *= std::exp(-p.damping * dt);
            p.position += p.velocity * dt;
            break;
        }
        case Method::VelocityVerlet: {
            Vec3 a_old = external * p.inv_mass();
            Vec3 v_half = p.velocity + a_old * (dt * 0.5);
            p.position += v_half * dt;
            Vec3 a_new = (p.force + detail::evaluate(fields, p.position, v_half, time + dt)) * p.inv_mass();
            p.velocity = v_half + a_new * (dt * 0.5);
            p.acceleration = a_new;
            break;
        }
        case Method::RK4: {
            Vec3 k1v = (p.force + detail::evaluate(fields, p.position, p.velocity, time)) * p.inv_mass();
            Vec3 k1p = p.velocity;
            Vec3 p2 = p.position + k1p * (dt * 0.5);
            Vec3 v2 = p.velocity + k1v * (dt * 0.5);
            Vec3 k2v = (p.force + detail::evaluate(fields, p2, v2, time + dt * 0.5)) * p.inv_mass();
            Vec3 k2p = v2;
            Vec3 p3 = p.position + k2p * (dt * 0.5);
            Vec3 v3 = p.velocity + k2v * (dt * 0.5);
            Vec3 k3v = (p.force + detail::evaluate(fields, p3, v3, time + dt * 0.5)) * p.inv_mass();
            Vec3 k3p = v3;
            Vec3 p4 = p.position + k3p * dt;
            Vec3 v4 = p.velocity + k3v * dt;
            Vec3 k4v = (p.force + detail::evaluate(fields, p4, v4, time + dt)) * p.inv_mass();
            Vec3 k4p = v4;
            p.position += (k1p + k2p * 2.0 + k3p * 2.0 + k4p) * (dt / 6.0);
            p.velocity += (k1v + k2v * 2.0 + k3v * 2.0 + k4v) * (dt / 6.0);
            p.acceleration = k1v;
            break;
        }
    }
    if (p.damping > 0.0 && method != Method::SemiImplicitEuler && method != Method::VelocityVerlet) p.velocity *= std::exp(-p.damping * dt);
    p.force = Vec3{0.0, 0.0, 0.0};
}

struct ParticleSystem {
    std::vector<Particle> particles;
    std::vector<ForceField> fields;
    double time = 0.0;
    Method method = Method::SemiImplicitEuler;
    ParticleSystem& add(Particle p) {
        particles.push_back(std::move(p));
        return *this;
    }
    ParticleSystem& add_field(ForceField f) {
        fields.push_back(std::move(f));
        return *this;
    }
    ParticleSystem& use(Method m) {
        method = m;
        return *this;
    }

    void step(double dt) {
        for (auto& p : particles) integrate(p, dt, fields, time, method);
        time += dt;
    }

    std::size_t alive_count() const {
        std::size_t n = 0;
        for (const auto& p : particles) if (p.alive) ++n;
        return n;
    }
};
}
}
