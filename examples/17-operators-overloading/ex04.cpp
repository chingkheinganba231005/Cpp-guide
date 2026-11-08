#include <iostream>

class Counter {
public:
    int operator()() { return ++count_; }
private:
    int count_ = 0;
};

int main() {
    Counter next_id;
    std::cout << next_id() << ' ' << next_id() << ' ' << next_id() << '\n';
}
