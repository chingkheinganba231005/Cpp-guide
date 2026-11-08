#include <future>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    std::vector<double> data(8'000'000, 0.5);
    auto mid = data.begin() + data.size() / 2;
    auto left = std::async(std::launch::async, [&] { return std::accumulate(data.begin(), mid, 0.0); });   // another thread
    double right = std::accumulate(mid, data.end(), 0.0);                                                  // this thread
    std::cout << left.get() + right << '\n';      // get() waits for the other half
}
