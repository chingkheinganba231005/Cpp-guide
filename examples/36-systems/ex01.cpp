#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main() {
    fs::create_directories("data/logs");
    {
        std::ofstream out("data/logs/run.csv");                // RAII: closed at the end of the block
        out << "step,loss\n";
        for (int i = 0; i < 5; ++i) out << i << ',' << 1.0 / (i + 1) << '\n';
    }
    std::ifstream in("data/logs/run.csv");
    std::string line;
    int lines = 0;
    while (std::getline(in, line)) ++lines;
    std::cout << "read " << lines << " lines\n";
    for (const auto& entry : fs::recursive_directory_iterator("data"))
        if (entry.is_regular_file()) std::cout << entry.path().string() << "  " << entry.file_size() << " bytes\n";
    fs::remove_all("data");
}
