#include <memory>
#include <vector>

struct Base { ~Base() {} };                         // NOT virtual: a bug
struct Derived : Base { std::vector<int> big = std::vector<int>(1000); };

int main() {
    std::unique_ptr<Base> p = std::make_unique<Derived>();
}   // only ~Base runs; Derived's vector is never freed (and the size passed to delete is wrong)
