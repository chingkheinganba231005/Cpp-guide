#include <chrono>
#include <iostream>
#include "ecs.hpp"

struct Position { float x, y; };
struct Velocity { float x, y; };

int main() {
    World<Position, Velocity> w;
    const int N = 1'000'000;
    for (int i = 0; i < N; ++i) { Entity e = w.create(); w.add(e, Position{0, 0}); w.add(e, Velocity{1, 2}); }
    auto t0 = std::chrono::steady_clock::now();
    const int FRAMES = 100;
    for (int f = 0; f < FRAMES; ++f)
        w.each<Position, Velocity>([](Entity, Position& p, Velocity& v) { p.x += v.x * 0.01f; p.y += v.y * 0.01f; });
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count() / FRAMES;
    std::cout << N << " entities moved in " << ms << " ms per frame (budget at 60 fps: 16.7 ms)\n";
    std::cout << "entity 0 at " << w.store<Position>().get(0).x << ", " << w.store<Position>().get(0).y << '\n';
}
