#include "raster.hpp"

int main() {
    Image img(320, 240);
    img.fill({0.08f, 0.09f, 0.12f});
    V2 a{160, 20}, b{290, 220}, c{30, 200};
    Color ca{1, 0.2f, 0.2f}, cb{0.2f, 1, 0.3f}, cc{0.25f, 0.4f, 1};
    raster_triangle(img, a, b, c, [&](int x, int y, float w0, float w1, float w2) {
        img.at(x, y) = Color{w0 * ca.r + w1 * cb.r + w2 * cc.r,      // blend the corner colours
                             w0 * ca.g + w1 * cb.g + w2 * cc.g,
                             w0 * ca.b + w1 * cb.b + w2 * cc.b};
    });
    img.save_ppm("triangle.ppm");
}
