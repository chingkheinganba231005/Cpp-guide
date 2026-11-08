#include <array>
#include <iostream>
#include <vector>

struct Tensor2D {
    std::vector<float> storage;
    std::array<std::size_t, 2> shape, stride;
    std::size_t offset = 0;
    float& operator()(std::size_t i, std::size_t j) { return storage[offset + i * stride[0] + j * stride[1]]; }
    Tensor2D transposed() const { return {storage, {shape[1], shape[0]}, {stride[1], stride[0]}, offset}; }   // no data moved
};

int main() {
    Tensor2D a{{1, 2, 3, 4, 5, 6}, {2, 3}, {3, 1}};      // a 2x3 matrix, row-major
    Tensor2D t = a.transposed();                          // 3x2: the same numbers, strides swapped
    for (std::size_t i = 0; i < t.shape[0]; ++i) { for (std::size_t j = 0; j < t.shape[1]; ++j) std::cout << t(i, j) << ' '; std::cout << '\n'; }
}
