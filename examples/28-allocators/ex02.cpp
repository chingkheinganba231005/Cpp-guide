#include <chrono>
#include <iostream>
#include "arena.hpp"

struct Node { float x, y; Node* next; };

int main() {
    constexpr int N = 1'000'000, FRAMES = 20;
    auto t0 = std::chrono::steady_clock::now();
    for (int f = 0; f < FRAMES; ++f) {                       // general-purpose heap: new + delete each object
        Node* head = nullptr;
        for (int i = 0; i < N; ++i) head = new Node{1.f * i, 2.f, head};
        while (head) { Node* n = head->next; delete head; head = n; }
    }
    auto t1 = std::chrono::steady_clock::now();
    Arena arena(sizeof(Node) * N + 64);
    float check = 0;
    for (int f = 0; f < FRAMES; ++f) {                       // arena: bump-allocate, then free everything in O(1)
        Node* head = nullptr;
        for (int i = 0; i < N; ++i) head = arena.make<Node>(1.f * i, 2.f, head);
        check += head->x;
        arena.reset();
    }
    auto t2 = std::chrono::steady_clock::now();
    auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
    std::cout << "new/delete: " << ms(t0, t1) << " ms\narena:      " << ms(t1, t2) << " ms (" << static_cast<int>(ms(t0, t1) / ms(t1, t2)) << "x faster)\n";
    std::cout << "check " << check << '\n';
}
