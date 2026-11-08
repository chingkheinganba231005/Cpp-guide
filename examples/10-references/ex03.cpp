#include <chrono>
#include <iostream>
#include <vector>

double sum_by_value(std::vector<double> v) {         // copies the whole vector
    double s = 0; for (double x : v) s += x; return s;
}
double sum_by_ref(const std::vector<double>& v) {    // just refers to the caller's vector
    double s = 0; for (double x : v) s += x; return s;
}

int main() {
    std::vector<double> big(5'000'000, 1.0);
    auto time = [&](auto f) {
        auto t0 = std::chrono::steady_clock::now();
        double s = 0;
        for (int i = 0; i < 20; ++i) s += f(big);
        auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        return std::pair{s, ms};
    };
    auto [s1, t1] = time(sum_by_value);
    auto [s2, t2] = time(sum_by_ref);
    std::cout << "by value: " << s1 << " in " << t1 << " ms\n";
    std::cout << "by const&: " << s2 << " in " << t2 << " ms\n";
    std::cout << "copying made it about " << static_cast<int>(t1 / t2 + 0.5) << "x slower\n";
}
