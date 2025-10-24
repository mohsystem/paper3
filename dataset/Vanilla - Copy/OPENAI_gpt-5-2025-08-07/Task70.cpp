#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>

std::string readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return std::string();
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            std::string content = readFile(argv[i]);
            if (!content.empty()) {
                std::cout << content;
                if (content.empty() || content.back() != '\n') std::cout << "\n";
            } else {
                std::cerr << "ERROR reading " << argv[i] << "\n";
            }
        }
        return 0;
    }

    // 5 test cases using files in the current directory if no command-line args provided
    std::vector<std::string> files;
    for (int i = 1; i <= 5; ++i) {
        std::string name = "task70_test_cpp_" + std::to_string(i) + ".txt";
        std::ofstream out(name, std::ios::binary);
        out << "Test case " << i << " content.\n";
        out.close();
        files.push_back(name);
    }

    for (const auto& f : files) {
        std::string content = readFile(f);
        std::cout << content;
    }

    for (const auto& f : files) {
        std::remove(f.c_str());
    }

    return 0;
}