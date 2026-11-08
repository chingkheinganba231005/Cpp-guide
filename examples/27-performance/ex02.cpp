#include <chrono>
#include <iostream>
#include <vector>

struct ParticleAoS { float x, y, z, vx, vy, vz, mass, charge, r, g, b, a, age, life, id, flags; };   // 64 bytes

struct ParticlesSoA {                                       // one array per field
    std::vector<float> x, y, z, vx, vy, vz;
    explicit ParticlesSoA(size_t n) : x(n), y(n), z(n), vx(n, 1), vy(n, 1), vz(n, 1) {}
};

int main() {
    const size_t n = 4'000'000;
    const float dt = 0.016f;
    std::vector<ParticleAoS> aos(n);
    for (auto& p : aos) { p.vx = p.vy = p.vz = 1; }
    ParticlesSoA soa(n);

    auto t0 = std::chrono::steady_clock::now();
    for (int rep = 0; rep < 10; ++rep)
        for (auto& p : aos) { p.x += p.vx * dt; p.y += p.vy * dt; p.z += p.vz * dt; }     // uses 24 of every 64 bytes loaded
    auto t1 = std::chrono::steady_clock::now();
    for (int rep = 0; rep < 10; ++rep)
        for (size_t i = 0; i < n; ++i) { soa.x[i] += soa.vx[i] * dt; soa.y[i] += soa.vy[i] * dt; soa.z[i] += soa.vz[i] * dt; }
    auto t2 = std::chrono::steady_clock::now();

    auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
    std::cout << "AoS: " << ms(t0, t1) << " ms, SoA: " << ms(t1, t2) << " ms\n";
    std::cout << "check: " << aos[7].x << ' ' << soa.x[7] << '\n';
}
