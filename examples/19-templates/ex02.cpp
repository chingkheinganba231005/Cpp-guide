#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>

template <typename T, std::size_t Capacity>      // a type parameter AND a value parameter
class FixedStack {
public:
    void push(const T& value) {
        if (size_ == Capacity) throw std::overflow_error("stack full");
        items_[size_++] = value;
    }
    T pop() {
        if (size_ == 0) throw std::underflow_error("stack empty");
        return items_[--size_];
    }
    bool empty() const { return size_ == 0; }
    std::size_t size() const { return size_; }
private:
    T items_[Capacity]{};
    std::size_t size_ = 0;
};

int main() {
    FixedStack<int, 4> s;                 // no heap at all: the storage is inside the object
    for (int i = 1; i <= 4; ++i) s.push(i * i);
    while (!s.empty()) std::cout << s.pop() << ' ';
    std::cout << '\n';
    FixedStack<std::string, 2> names;
    names.push("ada"); names.push("bjarne");
    std::cout << names.pop() << ", sizeof(FixedStack<int,4>) = " << sizeof(FixedStack<int, 4>) << '\n';
}
