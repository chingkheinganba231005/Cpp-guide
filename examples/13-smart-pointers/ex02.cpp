#include <memory>

int main() {
    auto a = std::make_unique<int>(5);
    auto b = a;          // error: copying would mean two owners
}
