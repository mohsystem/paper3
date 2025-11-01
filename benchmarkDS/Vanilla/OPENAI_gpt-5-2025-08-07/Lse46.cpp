#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

std::string img(const std::string& name) {
    std::filesystem::path p = std::filesystem::path("images") / name;
    std::ifstream in(p, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open file: " + p.string());
    }
    std::vector<char> buf((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());
    return std::string(buf.begin(), buf.end());
}

int main() {
    namespace fs = std::filesystem;
    fs::create_directories("images");
    std::vector<std::pair<std::string, std::string>> samples = {
        {"t1.txt", "alpha"},
        {"t2.txt", "bravo charlie"},
        {"t3.txt", "delta echo foxtrot"},
        {"t4.txt", "golf hotel india juliet"},
        {"t5.txt", "kilo lima mike november oscar"}
    };
    for (auto& kv : samples) {
        std::ofstream out(fs::path("images") / kv.first, std::ios::binary);
        out.write(kv.second.data(), (std::streamsize)kv.second.size());
    }

    // 5 test cases
    for (auto& kv : samples) {
        try {
            std::string data = img(kv.first);
            std::cout << "Read " << data.size() << " bytes from images/" << kv.first
                      << ": " << data.substr(0, 16) << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    return 0;
}