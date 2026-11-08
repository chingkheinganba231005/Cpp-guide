#include <cstddef>

class Buffer {                    // BROKEN: owns memory but uses the default copy
public:
    explicit Buffer(std::size_t n) : data_(new float[n]), size_(n) {}
    ~Buffer() { delete[] data_; }
private:
    float* data_;
    std::size_t size_;
};

int main() {
    Buffer a(100);
    Buffer b = a;     // copies the pointer: a and b now share one block
}                     // b's destructor frees it, then a's destructor frees it AGAIN
