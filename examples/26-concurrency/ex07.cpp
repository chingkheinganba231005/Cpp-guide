#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

int main() {
    std::queue<int> jobs;
    std::mutex m;
    std::condition_variable cv;
    bool done = false;

    std::jthread worker([&] {
        int processed = 0;
        while (true) {
            std::unique_lock lock(m);
            cv.wait(lock, [&] { return !jobs.empty() || done; });   // sleeps until notified AND the condition holds
            if (jobs.empty() && done) break;
            int job = jobs.front(); jobs.pop();
            lock.unlock();                                           // do the work outside the lock
            processed += job;
        }
        std::cout << "worker processed total " << processed << '\n';
    });

    for (int i = 1; i <= 100; ++i) {
        { std::lock_guard lock(m); jobs.push(i); }
        cv.notify_one();
    }
    { std::lock_guard lock(m); done = true; }
    cv.notify_one();
}
