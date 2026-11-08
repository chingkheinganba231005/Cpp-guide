#include <iostream>
#include <thread>
#include <vector>

int main() {
    long long counter = 0;                      // shared, unprotected: a data race (undefined behaviour!)
    {
        std::vector<std::jthread> ts;
        for (int t = 0; t < 4; ++t)
            ts.emplace_back([&counter] { for (int i = 0; i < 1'000'000; ++i) { counter = counter + 1; asm volatile("" ::: "memory"); } });
    }
    std::cout << "expected 4000000, got " << counter << '\n';
}
