#include <cstdio>
#include <iostream>
#include <memory>

int main() {
    // fclose is called automatically when the unique_ptr dies, even if an exception is thrown
    auto closer = [](std::FILE* fp) { std::fclose(fp); };
    std::unique_ptr<std::FILE, decltype(closer)> f(std::fopen("demo.txt", "w"), closer);
    if (f) std::fputs("written through a RAII-managed C FILE*\n", f.get());
    std::cout << "file " << (f ? "opened" : "failed") << '\n';
}
