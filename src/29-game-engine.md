A game engine is a program that updates a simulated world many times a second and draws it. In this project you build the core that every engine has: a **fixed-timestep game loop**, an **entity-component system (ECS)** with cache-friendly storage, and **systems** for movement, gravity and collisions. Rendering here is a small ASCII frame; chapter 29 builds a real rasteriser. Swapping in SDL, raylib or Vulkan for a window and graphics is the next step, and the core stays the same.

## The game loop and the fixed timestep

Frames don't take equal time: one takes 14 ms, the next 19 ms. If physics used the variable frame time, the simulation would differ from run to run and become unstable at low frame rates. The standard solution, from Glenn Fiedler's classic article *Fix Your Timestep!*, is to advance physics in **fixed steps** (say 1/120 s) and render as often as possible:

```cpp norun
double accumulator = 0;
auto previous = clock::now();
while (running) {
    auto now = clock::now();
    accumulator += seconds(now - previous);           // real time that has passed
    previous = now;
    process_input();
    while (accumulator >= DT) {                       // catch up in fixed steps
        update_physics(DT);
        accumulator -= DT;
    }
    render(accumulator / DT);                         // interpolate between the last two physics states
}
```

The physics becomes **deterministic**: the same inputs always give the same world. Multiplayer, replays and testing depend on that.

## Entity-component system: data-oriented game objects

The classic object-oriented approach (`class Enemy : public Character : public GameObject` with virtual `update()`) scatters objects across the heap, makes every update a virtual call, and tangles behaviour in deep hierarchies. An **ECS** separates the three concerns:

- An **entity** is just an ID (an integer).
- A **component** is plain data attached to an entity: `Position`, `Velocity`, `Collider`, `Health`.
- A **system** is a function that processes every entity that has a given set of components.

Components of each type are stored **contiguously**, so systems stream through memory (chapter 26). This is how Unity's DOTS, Bevy, EnTT and many AAA engines are organised.

### Sparse-set component storage

Each component type gets a **sparse set**: a dense, contiguous array of components, plus an index from entity ID to position in that array. Add, remove and lookup are O(1), and iteration is a linear scan of contiguous memory:

```cpp file=ecs.hpp
#pragma once
#include <cstdint>
#include <limits>
#include <tuple>
#include <vector>

using Entity = std::uint32_t;

template <typename C>
class Storage {                                   // a sparse set for one component type
public:
    void add(Entity e, C c) {
        if (e >= sparse_.size()) sparse_.resize(e + 1, NONE);
        sparse_[e] = static_cast<std::uint32_t>(dense_.size());
        dense_.push_back(c);
        owners_.push_back(e);
    }
    void remove(Entity e) {                       // O(1): move the last element into the hole
        std::uint32_t i = sparse_[e], last = static_cast<std::uint32_t>(dense_.size() - 1);
        dense_[i] = dense_[last]; owners_[i] = owners_[last]; sparse_[owners_[i]] = i;
        dense_.pop_back(); owners_.pop_back(); sparse_[e] = NONE;
    }
    bool has(Entity e) const { return e < sparse_.size() && sparse_[e] != NONE; }
    C& get(Entity e) { return dense_[sparse_[e]]; }
    std::vector<C>& data() { return dense_; }
    const std::vector<Entity>& owners() const { return owners_; }
private:
    static constexpr std::uint32_t NONE = std::numeric_limits<std::uint32_t>::max();
    std::vector<C> dense_;                        // the components, contiguous
    std::vector<Entity> owners_;                  // which entity owns each dense element
    std::vector<std::uint32_t> sparse_;           // entity -> index into dense_
};

template <typename... Cs>
class World {                                     // all component storages, one per type
public:
    Entity create() { return next_++; }
    template <typename C> void add(Entity e, C c) { std::get<Storage<C>>(stores_).add(e, c); }
    template <typename C> void remove(Entity e) { std::get<Storage<C>>(stores_).remove(e); }
    template <typename C> Storage<C>& store() { return std::get<Storage<C>>(stores_); }

    template <typename First, typename... Rest, typename F>
    void each(F&& f) {                            // visit every entity that has First and all of Rest
        auto& first = store<First>();
        auto& owners = first.owners();
        for (std::size_t i = 0; i < owners.size(); ++i) {
            Entity e = owners[i];
            if ((store<Rest>().has(e) && ...)) f(e, first.data()[i], store<Rest>().get(e)...);
        }
    }
private:
    std::tuple<Storage<Cs>...> stores_;
    Entity next_ = 0;
};
```

## The simulation

Bouncing balls with gravity, a floor, walls and a lifetime component. Systems are plain functions that run over the components they need:

```cpp run with=ecs.hpp
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
```

The balls fall, bounce with energy loss, rebound from the walls, and one expires after a second, all processed by systems streaming over contiguous arrays.

## Scaling up: how fast is the ECS?

```cpp run with=ecs.hpp
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
```

## From here to a real engine

| Next piece | What to learn |
|---|---|
| A window, input and graphics | SDL3 or GLFW for the window; OpenGL, Vulkan, Metal or WebGPU for drawing; or raylib for simplicity |
| Rendering | chapter 29's rasteriser explains what the GPU does; then shaders, meshes, textures and cameras |
| Physics | broad phase (spatial hashing, BVH), narrow phase (shape tests), impulse-based resolution; or Box2D / Jolt Physics |
| Assets | loading meshes (glTF), images (stb_image), audio (miniaudio) |
| Tools and scripting | an editor with Dear ImGui; scripting with Lua |
| Multithreading | a job system: split systems into tasks across cores (chapter 25) |

:::exercise Add a collision response between balls
Add a system that checks every pair of balls (O(n²) is fine for a few) and, when the distance between centres is less than the sum of the radii, swaps their velocity components along the line between them (an elastic collision of equal masses). For many balls, first sort them into a uniform grid (spatial hashing) so you only check neighbours.
---solution---
```cpp norun
void ball_collisions(Game& g) {
    auto& pos = g.store<Position>();
    auto& owners = pos.owners();
    for (size_t i = 0; i < owners.size(); ++i)
        for (size_t j = i + 1; j < owners.size(); ++j) {
            Entity a = owners[i], b = owners[j];
            auto& pa = pos.get(a); auto& pb = pos.get(b);
            float dx = pb.x - pa.x, dy = pb.y - pa.y, d2 = dx * dx + dy * dy;
            float r = g.store<Ball>().get(a).radius + g.store<Ball>().get(b).radius;
            if (d2 > 0 && d2 < r * r) {
                float d = std::sqrt(d2), nx = dx / d, ny = dy / d;
                auto& va = g.store<Velocity>().get(a); auto& vb = g.store<Velocity>().get(b);
                float pa_n = va.x * nx + va.y * ny, pb_n = vb.x * nx + vb.y * ny;   // velocity along the normal
                va.x += (pb_n - pa_n) * nx; va.y += (pb_n - pa_n) * ny;            // exchange the normal components
                vb.x += (pa_n - pb_n) * nx; vb.y += (pa_n - pb_n) * ny;
            }
        }
}
```
:::

::quiz
? Why do engines advance physics in fixed timesteps?
+ For deterministic, stable simulation independent of the frame rate
- Because rendering is fixed
- To save memory
> Variable steps make results differ between runs and can explode at low frame rates.
? In an ECS, what is an entity?
+ Just an ID; its data lives in component arrays
- A class with virtual update()
- A thread
> Behaviour lives in systems, data in contiguous component storage.
::
