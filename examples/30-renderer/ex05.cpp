#include <array>
#include <cmath>
#include <limits>
#include <vector>
#include "raster.hpp"

struct V3 { float x, y, z; };
V3 operator-(V3 a, V3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
V3 cross(V3 a, V3 b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
float dot(V3 a, V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
V3 normalize(V3 v) { float l = std::sqrt(dot(v, v)); return {v.x / l, v.y / l, v.z / l}; }
V3 rotate_y(V3 v, float a) { return {v.x * std::cos(a) + v.z * std::sin(a), v.y, -v.x * std::sin(a) + v.z * std::cos(a)}; }
V3 rotate_x(V3 v, float a) { return {v.x, v.y * std::cos(a) - v.z * std::sin(a), v.y * std::sin(a) + v.z * std::cos(a)}; }

int main() {
    const int W = 480, H = 360;
    Image img(W, H);
    for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) img.at(x, y) = Color{0.06f, 0.07f, 0.1f + 0.15f * y / H};
    std::vector<float> zbuf(W * H, std::numeric_limits<float>::infinity());

    // a unit cube: 8 corners, 12 triangles (2 per face), counter-clockwise when seen from outside
    std::array<V3, 8> v{{{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}}};
    std::array<std::array<int, 3>, 12> tris{{{0,2,1},{0,3,2},{4,5,6},{4,6,7},{0,1,5},{0,5,4},{3,7,6},{3,6,2},{0,4,7},{0,7,3},{1,2,6},{1,6,5}}};
    std::array<Color, 6> face_color{{{0.9f,0.35f,0.3f},{0.3f,0.75f,0.45f},{0.35f,0.5f,0.95f},{0.95f,0.8f,0.3f},{0.8f,0.4f,0.9f},{0.3f,0.85f,0.9f}}};
    V3 light = normalize({-0.5f, 0.8f, -0.6f});

    for (int which = 0; which < 2; ++which) {                                // draw two cubes
        {
            float ox = which ? 1.9f : -1.6f, oz = which ? 9.0f : 7.0f, ang = which ? 0.5f : 0.8f;
            std::array<V3, 8> world, screen;
            for (int i = 0; i < 8; ++i) {
                V3 p = rotate_x(rotate_y(v[i], ang), -0.45f);                  // 1. model rotation
                world[i] = {p.x + ox, p.y, p.z + oz};                         //    and placement
                float f = 1.2f * H;                                           // 2. perspective: divide by depth
                screen[i] = {W / 2 + f * world[i].x / world[i].z, H / 2 - f * world[i].y / world[i].z, world[i].z};   // 3. viewport
            }
            for (int t = 0; t < 12; ++t) {
                auto [i0, i1, i2] = tris[t];
                V3 n = normalize(cross(world[i1] - world[i0], world[i2] - world[i0]));
                if (dot(n, world[i0]) >= 0) continue;                         // back-face culling: facing away from the camera
                float lambert = std::max(0.0f, dot(n, light)) * 0.85f + 0.15f; // 6. diffuse + ambient light
                Color base = face_color[t / 2];
                raster_triangle(img, {screen[i0].x, screen[i0].y}, {screen[i1].x, screen[i1].y}, {screen[i2].x, screen[i2].y},
                    [&](int x, int y, float w0, float w1, float w2) {
                        float z = w0 * screen[i0].z + w1 * screen[i1].z + w2 * screen[i2].z;   // interpolated depth
                        float& depth = zbuf[y * W + x];
                        if (z < depth) {                                      // 5. depth test
                            depth = z;
                            img.at(x, y) = Color{base.r * lambert, base.g * lambert, base.b * lambert};
                        }
                    });
            }
        }
    }
    img.save_ppm("cube.ppm");
}
