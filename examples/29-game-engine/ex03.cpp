#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "ecs.hpp"

struct Position { float x, y; };
struct Velocity { float x, y; };
struct Ball { float radius; char glyph; };
struct Lifetime { float seconds; };

using Game = World<Position, Velocity, Ball, Lifetime>;
constexpr float DT = 1.0f / 120.0f, W = 40, H = 12, GRAVITY = -30.f, BOUNCE = 0.8f;

void gravity_system(Game& g) { g.each<Velocity>([](Entity, Velocity& v) { v.y += GRAVITY * DT; }); }

void movement_system(Game& g) {
    g.each<Position, Velocity>([](Entity, Position& p, Velocity& v) { p.x += v.x * DT; p.y += v.y * DT; });
}

void collision_system(Game& g) {                  // floor and walls
    g.each<Position, Velocity, Ball>([](Entity, Position& p, Velocity& v, Ball& b) {
        if (p.y - b.radius < 0) { p.y = b.radius; v.y = -v.y * BOUNCE; }
        if (p.x - b.radius < 0) { p.x = b.radius; v.x = -v.x; }
        if (p.x + b.radius > W) { p.x = W - b.radius; v.x = -v.x; }
    });
}

void lifetime_system(Game& g, std::vector<Entity>& dead) {
    g.each<Lifetime>([&](Entity e, Lifetime& l) { l.seconds -= DT; if (l.seconds <= 0) dead.push_back(e); });
}

std::string render(Game& g) {                    // a tiny ASCII "renderer"
    std::vector<std::string> rows(static_cast<int>(H), std::string(static_cast<int>(W), ' '));
    g.each<Position, Ball>([&](Entity, Position& p, Ball& b) {
        int c = static_cast<int>(p.x), r = static_cast<int>(H - 1 - p.y);
        if (r >= 0 && r < H && c >= 0 && c < W) rows[r][c] = b.glyph;
    });
    std::string out = "+" + std::string(static_cast<int>(W), '-') + "+\n";
    for (auto& r : rows) out += "|" + r + "|\n";
    return out + "+" + std::string(static_cast<int>(W), '=') + "+\n";
}

int main() {
    Game g;
    const char glyphs[] = "o*@#%";
    for (int i = 0; i < 5; ++i) {
        Entity e = g.create();
        g.add(e, Position{4.0f + i * 7, 11.0f - 1.8f * i});
        g.add(e, Velocity{6.0f - i * 3, 0});
        g.add(e, Ball{0.5f, glyphs[i]});
        if (i == 4) g.add(e, Lifetime{1.0f});     // this one disappears after one second
    }
    int step = 0;
    for (float t : {0.0f, 0.5f, 1.5f}) {
        while (step * DT < t) {                   // fixed-timestep updates up to time t
            gravity_system(g); movement_system(g); collision_system(g);
            std::vector<Entity> dead;
            lifetime_system(g, dead);
            for (Entity e : dead) { g.remove<Position>(e); g.remove<Velocity>(e); g.remove<Ball>(e); g.remove<Lifetime>(e); }
            ++step;
        }
        std::cout << "t = " << t << " s (" << step << " physics steps, " << g.store<Ball>().data().size() << " balls)\n" << render(g);
    }
}
