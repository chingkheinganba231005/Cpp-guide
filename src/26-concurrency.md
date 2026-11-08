Your CPU has several cores, and to use them a program runs several **threads** at once. Concurrency is essential for game engines (physics, audio and rendering in parallel), ML libraries (parallel matrix maths), servers and trading systems. It is also the source of the hardest bugs in programming. This chapter teaches the tools and the dangers.

:::python
Python's GIL lets only one thread run Python code at a time, so CPU-heavy Python uses processes or NumPy instead. **C++ threads truly run in parallel** on separate cores. That makes them fast, and it means two threads can touch the same memory at the same instant.
:::

## Starting threads

```cpp run
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
```

`std::jthread` (C++20) **joins** (waits for the thread to finish) when it is destroyed. With the older `std::thread`, forgetting to call `join()` terminates the program.

## Data races: the core danger

If two threads access the same memory at the same time and at least one of them writes, without synchronisation, that is a **data race**, and it is undefined behaviour. The classic example is a shared counter: `counter++` looks like one operation but is really three steps (load, add, store). Step the two threads yourself and make an update disappear:

<div data-widget="race" data-title="Two threads, one counter: find an interleaving that loses an update"></div>

Now the real thing, with four threads each incrementing a million times:

```cpp run
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
```

The result is wrong, and different every run. **ThreadSanitizer** (`-fsanitize=thread`) detects races precisely, names both conflicting accesses, and exits with code 66 so automated tests fail:

```cpp run flags="-fsanitize=thread -g -O1" expect=crash
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
```

## Mutexes: one thread at a time

A **mutex** (mutual exclusion) lets only one thread at a time into a **critical section**. Lock it with an RAII guard so it is always released:

```cpp run
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
```

Correct, but slow: every increment fights over the lock. Good concurrent design **shares less**: let each thread work on its own data and combine the results at the end, as the first example did.

:::warn Deadlock
If thread 1 locks A then B, while thread 2 locks B then A, each can wait forever for the other. Always lock multiple mutexes in a consistent order, or lock them together with `std::scoped_lock lock(a, b);`, which avoids deadlock.
:::

## Atomics: lock-free shared variables

For a single shared number, `std::atomic` makes each operation indivisible, using special CPU instructions with no lock:

```cpp run
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    std::atomic<long long> counter{0};
    {
        std::vector<std::jthread> ts;
        for (int t = 0; t < 4; ++t)
            ts.emplace_back([&] { for (int i = 0; i < 1'000'000; ++i) counter.fetch_add(1, std::memory_order_relaxed); });
    }
    std::cout << "expected 4000000, got " << counter.load() << '\n';
}
```

## Memory order, briefly

CPUs and compilers reorder memory operations for speed. Atomics take a **memory order** argument that says which reorderings are forbidden:

- `memory_order_seq_cst` (the default): the simplest to reason about, and the slowest.
- `memory_order_acquire` / `memory_order_release`: the standard pattern for handing data from one thread to another. Everything written before a *release* store is visible to the thread that sees it with an *acquire* load. Chapter 34's lock-free queue relies on exactly this.
- `memory_order_relaxed`: atomicity only, no ordering. Fine for counters and statistics.

```cpp run
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
```

## Condition variables: waiting for something to happen

A thread that waits by spinning in a loop wastes a whole core. A **condition variable** lets it sleep until another thread signals:

```cpp run
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
```

## Tasks and futures

For “run this in the background and give me the result later”, use `std::async`, which returns a `std::future`:

```cpp run
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
```

The standard library also offers parallel algorithms (`std::reduce(std::execution::par, ...)`), backed by a threading library such as Intel TBB on GCC. Real engines and servers use a **thread pool**: a fixed set of worker threads that take tasks from a queue, so they don't pay the cost of creating threads (tens of microseconds each). The condition-variable example above is the core of one.

## The rules that keep you sane

1. **Share as little as possible.** Give each thread its own data; combine the results at the end.
2. **Protect shared mutable data** with a mutex (`std::lock_guard`, `std::scoped_lock`) or make it atomic.
3. **Immutable data needs no locks.** Data written before the threads start, and only read afterwards, is safe.
4. **Run tests with ThreadSanitizer.**
5. **Don't guess about performance**: measure (chapter 26). Locks are often fine; contention is the real enemy.

::quiz
? Two threads do `++x` on a shared int without synchronisation. What is it?
+ A data race: undefined behaviour, and updates can be lost
- Safe, because ++ is one operation
- A deadlock
> ++x is a load, an add and a store; another thread can interleave between them.
? What does std::lock_guard give you?
+ A mutex that is unlocked automatically when the guard goes out of scope
- A faster atomic
- A thread pool
> RAII for locks: no forgotten unlocks, even when exceptions are thrown.
? Which tool detects data races?
+ -fsanitize=thread
- -fsanitize=address
- -O3
> ThreadSanitizer tracks which thread accessed which memory, and when.
::
