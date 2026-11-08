#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    const char* path = "prices.bin";
    {                                                              // write 1 million doubles to a file
        std::FILE* f = std::fopen(path, "wb");
        for (int i = 0; i < 1'000'000; ++i) { double p = 100.0 + i * 0.001; std::fwrite(&p, sizeof p, 1, f); }
        std::fclose(f);
    }
    int fd = open(path, O_RDONLY);
    struct stat st{};
    fstat(fd, &st);
    void* mem = mmap(nullptr, static_cast<size_t>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);   // POSIX: Linux, macOS
    if (mem == MAP_FAILED) { std::perror("mmap"); return 1; }
    const double* prices = static_cast<const double*>(mem);       // the file IS an array now
    std::size_t n = static_cast<size_t>(st.st_size) / sizeof(double);
    double sum = 0;
    for (std::size_t i = 0; i < n; ++i) sum += prices[i];
    std::cout << n << " prices, mean " << sum / n << ", last " << prices[n - 1] << '\n';
    munmap(mem, static_cast<size_t>(st.st_size));
    close(fd);
    std::remove(path);
}
