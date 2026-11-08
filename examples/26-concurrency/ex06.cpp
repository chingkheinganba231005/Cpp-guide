#include <atomic>
#include <iostream>
#include <thread>

int payload = 0;                        // ordinary data
std::atomic<bool> ready{false};

int main() {
    std::jthread producer([] {
        payload = 42;                                      // 1. write the data
        ready.store(true, std::memory_order_release);      // 2. publish: everything above becomes visible...
    });
    std::jthread consumer([] {
        while (!ready.load(std::memory_order_acquire)) {}  // 3. ...to whoever observes the flag with acquire
        std::cout << "payload = " << payload << '\n';      // guaranteed to print 42
    });
}
