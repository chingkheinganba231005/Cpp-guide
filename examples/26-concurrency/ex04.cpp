#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int main() {
    long long counter = 0;
    std::mutex m;
    {
        std::vector<std::jthread> ts;
        for (int t = 0; t < 4; ++t)
            ts.emplace_back([&] {
                for (int i = 0; i < 1'000'000; ++i) {
                    std::lock_guard lock(m);       // locked here...
                    ++counter;
                }                                  // ...unlocked here, automatically
            });
    }
    std::cout << "expected 4000000, got " << counter << '\n';
}
