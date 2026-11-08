#include <iostream>
#include <vector>

int main() {
    const int rows = 3, cols = 4;
    std::vector<int> grid(rows * cols);
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            grid[r * cols + c] = r * 10 + c;      // row-major: rows stored one after another
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) std::cout << grid[r * cols + c] << '\t';
        std::cout << '\n';
    }
}
