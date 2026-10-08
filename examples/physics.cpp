#include "uranium/physics/physics.hpp"
#include "uranium/viz/viz.hpp"
#include <cstdio>
#include <vector>
using namespace uranium;
using namespace uranium::physics;
int main() {
    ParticleSystem sys;
    sys.add(Particle::moving({0, 0, 0}, {6.0, 8.0, 0.0}, 1.0)).add_field(gravity() + linear_drag(0.05)).use(Method::RK4);
    std::vector<float> sim_x, sim_y, ana_x, ana_y;
    float t = 0.0f;
    const float dt = 1.0f / 120.0f;
    for (int i = 0; i <= 400; ++i) {
        const Particle& p = sys.particles[0];
        if (i % 4 == 0) {
            sim_x.push_back(p.position.x);
            sim_y.push_back(p.position.y);
            ana_x.push_back(6.0f * t);
            ana_y.push_back(8.0f * t - 4.9f * t * t);
        }
        sys.step(dt);
        t += dt;
    }
    const Particle& p = sys.particles[0];
    std::printf("projectile at t=%.2f  pos=(%.3f, %.3f)  (drag pulls it below vacuum arc)\n", t, p.position.x, p.position.y);
    World world;
    world.add(Plane::at_point({0, 0, 0}, {0, 1, 0}));
    RigidBody ground = RigidBody::solid_box({12, 0.5, 12}, 8000.0, {0, -0.5, 0});
    ground.make_static();
    world.add(ground);
    int box0 = world.add(RigidBody::solid_box({0.5, 0.5, 0.5}, 200.0, {0, 3.0, 0}));
    int box1 = world.add(RigidBody::solid_box({0.5, 0.5, 0.5}, 200.0, {0, 4.1, 0}));
    int box2 = world.add(RigidBody::solid_box({0.5, 0.5, 0.5}, 200.0, {0, 5.2, 0}));
    int ball = world.add(RigidBody::solid_sphere(0.4, 150.0, {2.5, 5.0, 0}));
    world.body(ball).restitution = 0.75;
    for (int i = 0; i < 900; ++i) world.step(1.0 / 60.0);
    std::printf("stacked boxes  y = %.3f / %.3f / %.3f   (half-extents 0.5, expect ~0.5, ~1.5, ~2.5)\n", world.body(box0).position.y, world.body(box1).position.y, world.body(box2).position.y);
    std::printf("bouncing ball  y = %.3f  vy = %.3f  energy = %.2f\n", world.body(ball).position.y, world.body(ball).linear_velocity.y, world.total_energy());
    viz::Figure fig(900, 550);
    fig.title("Projectile motion");
    viz::Axes& ax = fig.axes();
    ax.plot(sim_x, sim_y, "with linear drag (RK4)").line_width = 2.5f;
    ax.plot(ana_x, ana_y, "vacuum (analytic)").line_width = 2.0f;
    ax.axhline(0.0f);
    ax.set_xlim(0.0f, 25.0f);
    ax.set_ylim(0.0f, 4.5f);
    ax.title("one throw, two models").xlabel("x [m]").ylabel("y [m]");
    ax.legend(viz::LegendLoc::UpperRight);
    int apex = 0;
    for (int i = 1; i < static_cast<int>(sim_y.size()); ++i) if (sim_y[static_cast<std::size_t>(i)] > sim_y[static_cast<std::size_t>(apex)]) apex = i;
    ax.annotate("apex", {sim_x[static_cast<std::size_t>(apex)], sim_y[static_cast<std::size_t>(apex)]}, {30.0f, -30.0f});
    bool png = fig.save_png("projectile.png", 1.5f);
    bool svg = fig.save_svg("projectile.svg");
    std::printf("wrote projectile.png (%d) and projectile.svg (%d)\n", (int)png, (int)svg);
    return 0;
}
