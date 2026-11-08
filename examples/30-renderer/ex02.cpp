#include "image.hpp"

int main() {
    Image img(320, 120);
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            img.at(x, y) = Color{x / 319.0f, y / 119.0f, 0.6f};      // red grows to the right, green downwards
    img.save_ppm("gradient.ppm");
}
