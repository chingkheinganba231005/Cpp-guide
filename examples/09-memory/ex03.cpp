#include <cstddef>
#include <iostream>

struct Particle {
    float x, y, z;     // 3 × 4 bytes
    int id;            // 4 bytes
};

int main() {
    Particle p{1.0f, 2.0f, 3.0f, 7};
    std::cout << "sizeof(Particle) = " << sizeof(Particle) << '\n';
    std::cout << "x at offset " << offsetof(Particle, x) << ", y at " << offsetof(Particle, y)
              << ", z at " << offsetof(Particle, z) << ", id at " << offsetof(Particle, id) << '\n';
    Particle many[1000];
    std::cout << "1000 particles = " << sizeof many << " bytes, back to back\n";
    (void)p;
}
