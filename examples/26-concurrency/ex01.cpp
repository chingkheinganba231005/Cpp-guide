#include <iostream>
#include <thread>
#include <vector>

void work(int id, long long n, long long& out) {
    long long s = 0;
    for (long long i = 0; i < n; ++i) s += i % 7;
    out = s;                                            // each thread writes its own slot: no sharing
    (void)id;
}

int main() {
    unsigned cores = std::thread::hardware_concurrency();
    std::cout << "hardware threads: " << cores << '\n';
    std::vector<long long> results(4);
    std::vector<std::jthread> threads;                  // jthread joins automatically when destroyed (RAII again)
    for (int i = 0; i < 4; ++i) threads.emplace_back(work, i, 10'000'000, std::ref(results[i]));
    threads.clear();                                    // wait for all four to finish
    long long total = 0;
    for (auto r : results) total += r;
    std::cout << "total " << total << '\n';
}
