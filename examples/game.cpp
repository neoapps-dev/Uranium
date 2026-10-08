#include "uranium/physics/physics.hpp"
#include <raylib.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
using namespace uranium::physics;
namespace {
constexpr double PPM = 90.0;
constexpr double OX = 640.0;
constexpr double OY = 470.0;
constexpr Vec3 CANNON{-6.2, 1.6, 0.0};
struct Game {
    World world;
    std::vector<bool> box_out;
    int balls = 6;
    int ball_idx = -1;
    double shot_time = 0.0;
    bool fired = false;
    bool won = false;
    bool lost = false;
};

int add_box(Game& g, Vec3 center) {
    int i = g.world.add(RigidBody::solid_box({0.45, 0.45, 0.45}, 150.0, center));
    g.world.body(i).restitution = 0.2;
    return i;
}

void build(Game& g) {
    g = Game();
    int platform = g.world.add(RigidBody::solid_box({1.6, 0.25, 0.25}, 1.0, {0.0, -0.25, 0.0}));
    g.world.body(platform).make_static();
    add_box(g, {-0.9, 0.45, 0.0});
    add_box(g, {0.0, 0.45, 0.0});
    add_box(g, {0.9, 0.45, 0.0});
    add_box(g, {-0.45, 1.35, 0.0});
    add_box(g, {0.45, 1.35, 0.0});
    add_box(g, {0.0, 2.25, 0.0});
    g.box_out.assign(6, false);
}

void fire(Game& g, Vec3 dir, double speed) {
    if (g.ball_idx >= 0) {
        g.world.remove_body(static_cast<std::size_t>(g.ball_idx));
        g.ball_idx = -1;
    }
    auto ball = RigidBody::solid_sphere(0.35, 2000.0, CANNON);
    ball.restitution = 0.55;
    ball.linear_velocity = dir * speed;
    g.ball_idx = g.world.add(ball);
    g.fired = true;
    g.shot_time = 0.0;
}

Vec3 aim_dir(Vector2 mouse) {
    double wx = (mouse.x - OX) / PPM;
    double wy = (OY - mouse.y) / PPM;
    Vec3 d = Vec3{wx, wy, 0.0} - CANNON;
    double len = d.norm();
    if (len < 1e-9) d = Vec3{1.0, 0.0, 0.0};
    else d = d * (1.0 / len);
    return d;
}

std::vector<Vector2> preview_path(Vec3 dir, double speed) {
    std::vector<Vector2> pts;
    ParticleSystem sys;
    sys.add(Particle::moving(CANNON, dir * speed)).add_field(gravity()).use(Method::RK4);
    for (int i = 0; i < 300; ++i) {
        if (sys.particles[0].position.y < -1.0) break;
        sys.step(1.0 / 60.0);
        const Vec3& q = sys.particles[0].position;
        pts.push_back({static_cast<float>(OX + q.x * PPM), static_cast<float>(OY - q.y * PPM)});
    }
    return pts;
}

float box_angle(const RigidBody& b) {
    Mat3 m = b.orientation.to_matrix();
    double rad = std::atan2(m(1, 0), m(0, 0));
    return static_cast<float>(rad * 57.29577951308232);
}

void draw_body(const RigidBody& b, Color fill, Color outline) {
    float px = static_cast<float>(OX + b.position.x * PPM);
    float py = static_cast<float>(OY - b.position.y * PPM);
    if (const SphereShape* s = std::get_if<SphereShape>(&b.shape)) {
        float r = static_cast<float>(s->radius * PPM);
        DrawCircleV({px, py}, r, fill);
        DrawCircleLinesV({px, py}, r, outline);
        return;
    }
    if (const BoxShape* bx = std::get_if<BoxShape>(&b.shape)) {
        float w = static_cast<float>(2.0 * bx->half_extents.x * PPM);
        float h = static_cast<float>(2.0 * bx->half_extents.y * PPM);
        float a = box_angle(b);
        Vector2 origin{w / 2.0f, h / 2.0f};
        DrawRectanglePro({px, py, w, h}, origin, a, fill);
        float rad = a * 0.017453292519943295;
        float c = std::cos(rad);
        float s = std::sin(rad);
        float hw = w * 0.5f;
        float hh = h * 0.5f;
        Vector2 corners[4]{
            {px + (hw * c - hh * s), py + (hw * s + hh * c)},
            {px + (-hw * c - hh * s), py + (-hw * s + hh * c)},
            {px + (-hw * c + hh * s), py + (-hw * s - hh * c)},
            {px + (hw * c + hh * s), py + (hw * s - hh * c)},
        };
        for (int k = 0; k < 4; ++k) DrawLineEx(corners[k], corners[(k + 1) % 4], 2.0f, outline);
        return;
    }
}

int count_out(const Game& g) {
    int n = 0;
    for (bool out : g.box_out) n += out ? 1 : 0;
    return n;
}
}

int main() {
    constexpr int win_w = 1280;
    constexpr int win_h = 720;
    InitWindow(win_w, win_h, "Uranium + Raylib");
    SetTargetFPS(60);
    Game game;
    build(game);
    double speed = 11.0;
    const double fixed_dt = 1.0 / 60.0;
    double accumulator = 0.0;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            build(game);
            speed = 11.0;
        }

        Vector2 mouse = GetMousePosition();
        Vec3 dir = aim_dir(mouse);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && game.balls > 0 && !game.won && !game.lost) {
            fire(game, dir, speed);
            game.balls--;
        }
        speed += static_cast<double>(GetMouseWheelMove()) * 0.5;
        if (speed < 6.0) speed = 6.0;
        if (speed > 16.0) speed = 16.0;
        accumulator += GetFrameTime();
        if (accumulator > 0.25) accumulator = 0.25;
        while (accumulator >= fixed_dt) {
            game.world.step(fixed_dt);
            if (game.fired) game.shot_time += fixed_dt;
            accumulator -= fixed_dt;
        }
        if (game.fired && game.ball_idx >= 0) {
            const auto& ball = game.world.bodies()[static_cast<std::size_t>(game.ball_idx)];
            if (ball.position.y < -4.0 || std::fabs(ball.position.x) > 14.0 || game.shot_time > 12.0) {
                game.world.remove_body(static_cast<std::size_t>(game.ball_idx));
                game.ball_idx = -1;
                game.fired = false;
            }
        }
        for (std::size_t i = 0; i < game.box_out.size(); ++i) {
            if (game.world.bodies()[i + 1].position.y < -0.6) game.box_out[i] = true;
        }

        int knocked = count_out(game);
        if (knocked == 6 && !game.won && !game.lost) {
            game.won = true;
            std::printf("level clear in %d shots, energy %.0f J\n", 6 - game.balls, game.world.total_energy());
        }
        if (game.balls <= 0 && !game.won && !game.lost) {
            game.lost = true;
            std::printf("out of balls, %d/6 knocked over\n", knocked);
        }

        int fps = GetFPS();
        std::string ball_text = "balls   " + std::to_string(game.balls);
        std::string knock_text = "knocked " + std::to_string(knocked) + "/6";
        std::string speed_text = "speed   " + std::to_string(static_cast<int>(speed));
        std::string energy_text = "energy  " + std::to_string(static_cast<int>(game.world.total_energy()));
        std::string scene_text = "";
        if (game.won) scene_text = "level clear - press R";
        if (game.lost) scene_text = "out of balls - press R";
        if (game.won || game.lost) {
            std::string winmark = game.won ? "LEVEL CLEAR!" : "OUT OF BALLS";
            DrawText(winmark.c_str(), win_w / 2 - MeasureText(winmark.c_str(), 48) / 2, 250, 48, RED);
            DrawText(scene_text.c_str(), win_w / 2 - MeasureText(scene_text.c_str(), 24) / 2, 320, 24, DARKGRAY);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (const auto& b : game.world.bodies()) {
            if (b.is_static) draw_body(b, Color{110, 116, 128, 255}, Color{60, 64, 72, 255});
            else if (b.mass > 1500.0) draw_body(b, Color{230, 92, 66, 255}, Color{120, 32, 20, 255});
            else draw_body(b, Color{240, 190, 90, 255}, Color{150, 100, 30, 255});
        }

        if (!game.won && !game.lost) {
            float sx = static_cast<float>(OX + CANNON.x * PPM);
            float sy = static_cast<float>(OY - CANNON.y * PPM);
            DrawCircleV({sx, sy}, 10.0f, Color{60, 64, 72, 255});
            DrawLineEx({sx, sy},
                       {static_cast<float>(sx + dir.x * 140.0f), static_cast<float>(sy - dir.y * 140.0f)},
                       2.0f, Color{60, 64, 72, 200});
            for (const Vector2& p : preview_path(dir, speed))
                DrawCircleV(p, 3.0f, Color{70, 120, 220, 160});
        }

        for (const auto& c : game.world.contacts()) DrawCircleV({static_cast<float>(OX + c.point.x * PPM), static_cast<float>(OY - c.point.y * PPM)}, 4.0f, Color{70, 200, 90, 160});
        int hud_y = 20;
        const int dy = 26;
        DrawText("click to fire  R = reset  wheel = power", 20, hud_y, 22, DARKGRAY);
        hud_y += dy + 8;
        DrawText(ball_text.c_str(), 20, hud_y, 22, DARKGRAY);
        hud_y += dy;
        DrawText(knock_text.c_str(), 20, hud_y, 22, DARKGRAY);
        hud_y += dy;
        DrawText(speed_text.c_str(), 20, hud_y, 22, DARKGRAY);
        hud_y += dy;
        DrawText(energy_text.c_str(), 20, hud_y, 22, DARKGRAY);
        hud_y += dy;
        DrawText(std::to_string(fps).c_str(), win_w - 60, 20, 22, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
