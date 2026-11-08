#include <algorithm>
#include <cstddef>
#include <iostream>
#include <utility>

class Buffer {
public:
    explicit Buffer(std::size_t n) : data_(new float[n]{}), size_(n) {}
    ~Buffer() { delete[] data_; }

    Buffer(const Buffer& o) : data_(new float[o.size_]), size_(o.size_) {          // copy constructor: deep copy
        std::copy(o.data_, o.data_ + size_, data_);
        std::cout << "  copy (" << size_ << " floats)\n";
    }
    Buffer& operator=(const Buffer& o) {                                           // copy assignment
        if (this != &o) { Buffer tmp(o); swap(tmp); }                             // copy-and-swap: simple and safe
        return *this;
    }
    Buffer(Buffer&& o) noexcept : data_(std::exchange(o.data_, nullptr)), size_(std::exchange(o.size_, 0)) {   // move constructor: steal
        std::cout << "  move (no allocation)\n";
    }
    Buffer& operator=(Buffer&& o) noexcept { Buffer tmp(std::move(o)); swap(tmp); return *this; }   // move assignment
    void swap(Buffer& o) noexcept { std::swap(data_, o.data_); std::swap(size_, o.size_); }

    std::size_t size() const { return size_; }
    float& operator[](std::size_t i) { return data_[i]; }
private:
    float* data_;
    std::size_t size_;
};

Buffer make_big() { Buffer b(1'000'000); b[0] = 1; return b; }   // returned without any copy (copy elision)

int main() {
    Buffer a(1000);
    std::cout << "b = a:\n";
    Buffer b = a;                 // copy: a stays valid
    std::cout << "c = std::move(a):\n";
    Buffer c = std::move(a);      // move: c steals a's block; a is left empty
    std::cout << "a.size() after move = " << a.size() << '\n';
    std::cout << "d = make_big():\n";
    Buffer d = make_big();        // no copy, no move printed: the object is built directly in d
    std::cout << "d.size() = " << d.size() << ", c.size() = " << c.size() << ", b.size() = " << b.size() << '\n';
}
