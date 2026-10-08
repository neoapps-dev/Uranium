#pragma once
#include "uranium/core/config.hpp"
#include "uranium/core/error.hpp"
#include "uranium/physics/collision.hpp"
#include "uranium/physics/particle.hpp"
#include "uranium/physics/rigid_body.hpp"
#include <cmath>
#include <vector>
namespace uranium {
namespace physics {
struct WorldSettings {
    Vec3 gravity{0.0, -9.81, 0.0};
    int substeps = 4;
    int solver_iterations = 8;
    double restitution_threshold = 1.0;
    double baumgarte = 0.2;
    double penetration_slop = 0.005;
    bool allow_sleep = false;
};

class World {
public:
    World() = default;
    explicit World(WorldSettings settings): m_settings(settings) {}
    URANIUM_NODISCARD WorldSettings& settings() { return m_settings; }
    URANIUM_NODISCARD const WorldSettings& settings() const { return m_settings; }
    URANIUM_NODISCARD std::vector<RigidBody>& bodies() { return m_bodies; }
    URANIUM_NODISCARD const std::vector<RigidBody>& bodies() const { return m_bodies; }
    URANIUM_NODISCARD std::vector<Particle>& particles() { return m_particles; }
    URANIUM_NODISCARD const std::vector<Particle>& particles() const { return m_particles; }
    URANIUM_NODISCARD std::vector<Plane>& planes() { return m_planes; }
    URANIUM_NODISCARD const std::vector<Plane>& planes() const { return m_planes; }
    URANIUM_NODISCARD const std::vector<Contact>& contacts() const { return m_contacts; }
    int add(RigidBody body) {
        body.force = Vec3{0.0, 0.0, 0.0};
        body.torque = Vec3{0.0, 0.0, 0.0};
        m_bodies.push_back(std::move(body));
        return static_cast<int>(m_bodies.size()) - 1;
    }

    int add(Particle p) {
        m_particles.push_back(std::move(p));
        return static_cast<int>(m_particles.size()) - 1;
    }

    void add(Plane p) { m_planes.push_back(p); }
    void add_field(ForceField f) { m_fields.push_back(std::move(f)); }
    void remove_body(std::size_t index) { m_bodies.erase(m_bodies.begin() + static_cast<long>(index)); }
    URANIUM_NODISCARD RigidBody& body(int index) { return m_bodies[static_cast<std::size_t>(index)]; }
    void step(double dt) {
        if (dt <= 0.0) return;
        std::vector<ForceField> fields;
        if (!m_particles.empty()) {
            fields.push_back(constant_force(m_settings.gravity));
            for (const auto& f : m_fields) fields.push_back(f);
        }
        int sub = std::max(1, m_settings.substeps);
        double h = dt / static_cast<double>(sub);
        for (int s = 0; s < sub; ++s) {
            for (auto& p : m_particles) integrate(p, h, fields, m_time, m_method);
            for (auto& b : m_bodies) {
                if (!b.is_static) b.force += m_settings.gravity * b.mass;
            }
            for (auto& b : m_bodies) b.integrate(h);
            m_contacts.clear();
            detect();
            for (int it = 0; it < m_settings.solver_iterations; ++it) solve();
            correct_positions();
            m_time += h;
        }
    }

    void set_method(Method m) { m_method = m; }
    URANIUM_NODISCARD double time() const { return m_time; }
    URANIUM_NODISCARD Vec3 total_momentum() const {
        Vec3 p{0.0, 0.0, 0.0};
        for (const auto& b : m_bodies) p += b.linear_momentum();
        for (const auto& q : m_particles) if (q.alive) p += q.momentum();
        return p;
    }

    URANIUM_NODISCARD double total_energy() const {
        double e = 0.0;
        for (const auto& b : m_bodies) {
            e += b.kinetic_energy();
            if (!b.is_static) e += -m_settings.gravity.y * b.mass * b.position.y;
        }
        for (const auto& q : m_particles) {
            if (!q.alive) continue;
            e += q.kinetic_energy();
            e += -m_settings.gravity.y * q.mass * q.position.y;
        }
        return e;
    }

private:
    void detect() {
        for (std::size_t i = 0; i < m_bodies.size(); ++i) {
            for (std::size_t j = i + 1; j < m_bodies.size(); ++j) {
                if (m_bodies[i].is_static && m_bodies[j].is_static) continue;
                std::vector<Contact> cs;
                bool hit = collide_pair(m_bodies[i], m_bodies[j], cs);
                if (hit) {
                    for (const auto& c : cs) {
                        if (c.depth < 0.0) continue;
                        Contact cc = c;
                        cc.a = static_cast<int>(i);
                        cc.b = static_cast<int>(j);
                        m_contacts.push_back(cc);
                    }
                }
            }
            for (const auto& plane : m_planes) {
                Contact c;
                bool hit = collide_body_plane(m_bodies[i], plane, c);
                if (hit) {
                    c.a = static_cast<int>(i);
                    c.b = -1;
                    m_contacts.push_back(c);
                }
            }
        }
    }

    bool collide_pair(const RigidBody& a, const RigidBody& b, std::vector<Contact>& out) const {
        const SphereShape* sa = std::get_if<SphereShape>(&a.shape);
        const SphereShape* sb = std::get_if<SphereShape>(&b.shape);
        const BoxShape* ba = std::get_if<BoxShape>(&a.shape);
        const BoxShape* bb = std::get_if<BoxShape>(&b.shape);
        if (sa && sb) {
            Contact c;
            bool hit = collide_sphere_sphere(a.position + sa->offset, sa->radius, b.position + sb->offset, sb->radius, c);
            if (hit) out.push_back(c);
            return hit;
        }
        if (sa && bb) {
            Contact c;
            bool hit = collide_sphere_box(a.position + sa->offset, sa->radius, b.position + bb->offset, bb->half_extents, b.orientation.to_matrix(), c);
            if (hit) {
                c.normal = c.normal * -1.0;
                out.push_back(c);
            }
            return hit;
        }
        if (ba && sb) {
            Contact c;
            bool hit = collide_sphere_box(b.position + sb->offset, sb->radius, a.position + ba->offset, ba->half_extents, a.orientation.to_matrix(), c);
            if (hit) out.push_back(c);
            return hit;
        }
        if (ba && bb) {
            return collide_obb_obb(a.position + ba->offset, ba->half_extents, a.orientation.to_matrix(), b.position + bb->offset, bb->half_extents, b.orientation.to_matrix(), out);
        }
        return false;
    }

    bool collide_body_plane(const RigidBody& body, const Plane& plane, Contact& out) const {
        if (const SphereShape* s = std::get_if<SphereShape>(&body.shape)) {
            return collide_sphere_plane(body.position + s->offset, s->radius, plane, out);
        }
        if (const BoxShape* bx = std::get_if<BoxShape>(&body.shape)) {
            std::vector<Contact> cs;
            bool hit = collide_box_plane(body.position + bx->offset, bx->half_extents, body.orientation.to_matrix(), plane, cs);
            if (!hit) return false;
            Contact deepest = cs[0];
            for (const auto& c : cs) if (c.depth > deepest.depth) deepest = c;
            out = deepest;
            return true;
        }
        return false;
    }

    void solve() {
        for (auto& c : m_contacts) {
            RigidBody* a = c.a >= 0 ? &m_bodies[static_cast<std::size_t>(c.a)] : nullptr;
            RigidBody* b = c.b >= 0 ? &m_bodies[static_cast<std::size_t>(c.b)] : nullptr;
            double inv_ma = a && !a->is_static ? a->inv_mass : 0.0;
            double inv_mb = b && !b->is_static ? b->inv_mass : 0.0;
            if (inv_ma + inv_mb == 0.0) continue;
            Vec3 ra = c.point - (a ? a->position : c.point);
            Vec3 rb = c.point - (b ? b->position : c.point);
            Vec3 va = a ? a->point_velocity(c.point) : Vec3{0.0, 0.0, 0.0};
            Vec3 vb = b ? b->point_velocity(c.point) : Vec3{0.0, 0.0, 0.0};
            Vec3 rv = vb - va;
            double vn = dot(rv, c.normal);
            if (vn > 0.0) continue;
            double e = 0.0;
            if (a) e = std::max(e, a->restitution);
            if (b) e = std::max(e, b->restitution);
            if (-vn < m_settings.restitution_threshold) e = 0.0;
            Mat3 ia = a ? a->inv_inertia_world() : Mat3::zero();
            Mat3 ib = b ? b->inv_inertia_world() : Mat3::zero();
            Vec3 ran = cross(ra, c.normal);
            Vec3 rbn = cross(rb, c.normal);
            double denom = inv_ma + inv_mb + dot(cross(ia * ran, ra), c.normal) + dot(cross(ib * rbn, rb), c.normal);
            if (denom < 1e-12) continue;
            double j = -(1.0 + e) * vn / denom;
            Vec3 impulse = c.normal * j;
            if (a && !a->is_static) a->apply_impulse(impulse * -1.0, c.point);
            if (b && !b->is_static) b->apply_impulse(impulse, c.point);
            va = a ? a->point_velocity(c.point) : Vec3{0.0, 0.0, 0.0};
            vb = b ? b->point_velocity(c.point) : Vec3{0.0, 0.0, 0.0};
            rv = vb - va;
            Vec3 tangent = rv - c.normal * dot(rv, c.normal);
            double tl = tangent.norm();
            if (tl < 1e-9) continue;
            tangent = tangent * (1.0 / tl);
            double mu = 0.0;
            if (a) mu = std::max(mu, a->friction);
            if (b) mu = std::max(mu, b->friction);
            double vt = dot(rv, tangent);
            Vec3 rat = cross(ra, tangent);
            Vec3 rbt = cross(rb, tangent);
            double denom_t = inv_ma + inv_mb + dot(cross(ia * rat, ra), tangent) + dot(cross(ib * rbt, rb), tangent);
            if (denom_t < 1e-12) continue;
            double jt = -vt / denom_t;
            jt = std::max(-mu * j, std::min(mu * j, jt));
            Vec3 friction_impulse = tangent * jt;
            if (a && !a->is_static) a->apply_impulse(friction_impulse * -1.0, c.point);
            if (b && !b->is_static) b->apply_impulse(friction_impulse, c.point);
        }
    }

    void correct_positions() {
        for (auto& c : m_contacts) {
            RigidBody* a = c.a >= 0 ? &m_bodies[static_cast<std::size_t>(c.a)] : nullptr;
            RigidBody* b = c.b >= 0 ? &m_bodies[static_cast<std::size_t>(c.b)] : nullptr;
            double inv_ma = a && !a->is_static ? a->inv_mass : 0.0;
            double inv_mb = b && !b->is_static ? b->inv_mass : 0.0;
            double inv_sum = inv_ma + inv_mb;
            if (inv_sum == 0.0) continue;
            double corr = std::max(c.depth - m_settings.penetration_slop, 0.0) * m_settings.baumgarte;
            Vec3 shift = c.normal * (corr / inv_sum);
            if (a && !a->is_static) a->position -= shift * inv_ma;
            if (b && !b->is_static) b->position += shift * inv_mb;
        }
    }

    WorldSettings m_settings;
    std::vector<RigidBody> m_bodies;
    std::vector<Particle> m_particles;
    std::vector<Plane> m_planes;
    std::vector<ForceField> m_fields;
    std::vector<Contact> m_contacts;
    Method m_method = Method::SemiImplicitEuler;
    double m_time = 0.0;
};
}
}
