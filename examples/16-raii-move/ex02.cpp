#include <cstdio>
#include <iostream>
#include <stdexcept>

class File {                           // a minimal RAII wrapper around a C FILE*
public:
    File(const char* path, const char* mode) : f_(std::fopen(path, mode)) {
        if (!f_) throw std::runtime_error("cannot open file");
        std::cout << "opened " << path << '\n';
    }
    ~File() { std::fclose(f_); std::cout << "closed\n"; }
    void write(const char* s) { std::fputs(s, f_); }

    File(const File&) = delete;              // copying a file handle makes no sense: forbid it
    File& operator=(const File&) = delete;
private:
    std::FILE* f_;
};

void save(bool fail) {
    File out("log.txt", "w");
    out.write("first line\n");
    if (fail) throw std::runtime_error("something went wrong");   // the file is still closed!
    out.write("second line\n");
}

int main() {
    try { save(true); } catch (const std::exception& e) { std::cout << "caught: " << e.what() << '\n'; }
}
