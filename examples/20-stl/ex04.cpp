#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

int main() {
    int limit = 5, hits = 0;
    std::vector<int> v{1, 7, 3, 9, 5};

    auto over = [limit](int x) { return x > limit; };       // capture limit by value (a copy)
    auto count = [&hits](int x) { if (x % 2) ++hits; };     // capture hits by reference
    std::ranges::for_each(v, count);
    std::cout << std::ranges::count_if(v, over) << " over " << limit << ", " << hits << " odd\n";

    auto make_adder = [](int n) { return [n](int x) { return x + n; }; };   // a lambda returning a lambda
    std::function<int(int)> add10 = make_adder(10);          // std::function can hold any callable
    std::cout << add10(5) << '\n';

    int counter = 0;
    auto tick = [counter]() mutable { return ++counter; };   // mutable: may modify its own copy
    std::cout << tick() << tick() << tick() << " (outer counter is still " << counter << ")\n";
}
