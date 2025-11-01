#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <random>

std::vector<unsigned char> img(const std::string& filePath) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in) return {};
    std::vector<unsigned char> data(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );
    return data;
}

int main() {
    // Create 5 test files
    std::vector<std::string> files = {
        "cpp_test1.bin", "cpp_test2.bin", "cpp_test3.bin", "cpp_test4.bin", "cpp_test5.bin"
    };

    {
        std::ofstream o(files[0], std::ios::binary);
        std::string s = "Hello C++";
        o.write(s.data(), s.size());
    }
    {
        std::ofstream o(files[1], std::ios::binary);
        unsigned char buf[] = {0x00, 0x01, 0x02, 0xFF, 0x10};
        o.write(reinterpret_cast<char*>(buf), sizeof(buf));
    }
    {
        std::ofstream o(files[2], std::ios::binary);
        std::string s;
        for (int i = 0; i < 10; ++i) s += "xyz";
        o.write(s.data(), s.size());
    }
    {
        std::ofstream o(files[3], std::ios::binary);
        // empty file
    }
    {
        std::ofstream o(files[4], std::ios::binary);
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(0, 255);
        std::vector<unsigned char> buf(1024);
        for (auto& b : buf) b = static_cast<unsigned char>(dist(rng));
        o.write(reinterpret_cast<char*>(buf.data()), buf.size());
    }

    // 5 test cases calling img()
    for (const auto& f : files) {
        auto data = img(f);
        std::cout << "Read " << f << " -> length=" << data.size();
        if (!data.empty()) {
            std::cout << ", first8=[";
            for (size_t i = 0; i < data.size() && i < 8; ++i) {
                if (i) std::cout << ", ";
                std::cout << static_cast<int>(data[i]);
            }
            std::cout << "]";
        }
        std::cout << "\n";
    }

    return 0;
}