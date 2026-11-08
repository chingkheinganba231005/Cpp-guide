#include <iostream>
#include <thread>

int main() {
    int counter = 0;
    {
        std::jthread a([&] { for (int i = 0; i < 1000; ++i) ++counter; });
        std::jthread b([&] { for (int i = 0; i < 1000; ++i) ++counter; });
    }
    std::cout << counter << '\n';
}
