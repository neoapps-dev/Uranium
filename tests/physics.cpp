#include "check.hpp"
#include <algorithm>
#include <cmath>
#include "uranium/physics/physics.hpp"
using namespace uranium;
using namespace uranium::physics;
int main() {
    {
        ParticleSystem sys;
        sys.add(Particle::at({0, 10, 0}, 1.0)).add_field(gravity()).use(Method::VelocityVerlet);
        for (int i = 0; i < 120; ++i) sys.step(1.0 / 60.0);
        const Particle& p = sys.particles[0];
        CHECK_NEAR(p.position.y, 10.0 - 0.5 * 9.81 * 4.0, 0.05);
        CHECK_NEAR(p.velocity.y, -9.81 * 2.0, 0.05);
    }
    {
        World world;
        world.add(Plane::at_point({0, 0, 0}, {0, 1, 0}));
        RigidBody ground = RigidBody::solid_box({10, 0.5, 10}, 5000.0, {0, -0.5, 0});
        ground.make_static();
        world.add(ground);
        int b1 = world.add(RigidBody::solid_sphere(0.5, 400.0, {0, 3, 0}));
        int b2 = world.add(RigidBody::solid_sphere(0.5, 400.0, {0, 5, 0}));
        world.body(b1).restitution = 0.2;
        world.body(b2).restitution = 0.2;
        for (int i = 0; i < 700; ++i) world.step(1.0 / 60.0);
        CHECK_NEAR(world.body(b1).position.y, 0.5, 0.05);
        CHECK_NEAR(world.body(b2).position.y, 1.5, 0.08);
        CHECK(std::fabs(world.body(b1).linear_velocity.y) < 0.2f);
    }
    {
        World world;
        world.add(Plane::at_point({0, 0, 0}, {0, 1, 0}));
        int box = world.add(RigidBody::solid_box({0.5, 0.5, 0.5}, 250.0, {0, 2, 0}));
        for (int i = 0; i < 500; ++i) world.step(1.0 / 60.0);
        CHECK_NEAR(world.body(box).position.y, 0.5, 0.05);
        CHECK_NEAR(world.body(box).orientation.w, 1.0, 0.1);
    }
    {
        RigidBody spin = RigidBody::solid_box({0.5, 0.5, 0.5}, 200.0, {0, 2, 0});
        for (int i = 0; i < 60; ++i) {
            spin.apply_force_at({0, 0, 40}, {0.5, 2, 0});
            spin.integrate(1.0 / 60.0);
        }
        CHECK(spin.angular_velocity.norm() > 0.1f);
    }
    {
        ParticleSystem sys;
        sys.add(Particle::moving({0, 0, 0}, {5, 5, 0}, 1.0))
            .add_field(gravity() + linear_drag(0.1))
            .use(Method::RK4);
        for (int i = 0; i < 200; ++i) sys.step(1.0 / 60.0);
        double t = 200.0 / 60.0;
        double k = 0.1;
        double x_ex = 5.0 * (1.0 - std::exp(-k * t)) / k;
        double y_ex = -9.81 / k * t + (5.0 + 9.81 / k) * (1.0 - std::exp(-k * t)) / k;
        CHECK_NEAR(sys.particles[0].position.x, x_ex, 1e-6);
        CHECK_NEAR(sys.particles[0].position.y, y_ex, 1e-6);
        CHECK(sys.particles[0].position.x > 3.0f);
    }
    {
        ParticleSystem sys;
        sys.add(Particle::at({1, 0, 0}, 1.0)).add_field(harmonic(4.0)).use(Method::VelocityVerlet);
        double max_err = 0.0;
        double t = 0.0;
        for (int i = 0; i < 1200; ++i) {
            sys.step(1.0 / 240.0);
            t += 1.0 / 240.0;
            max_err = std::max(max_err, std::fabs(sys.particles[0].position.x - std::cos(2.0 * t)));
        }
        CHECK(max_err < 0.001);
    }
    return 0;
}
