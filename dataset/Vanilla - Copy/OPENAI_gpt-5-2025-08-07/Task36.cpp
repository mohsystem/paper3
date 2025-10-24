#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::string readFile(const std::string& path) {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in) return std::string();
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main() {
    std::vector<std::string> filenames = {
        "task36_cpp_test_1.txt",
        "task36_cpp_test_2.txt",
        "task36_cpp_test_3.txt",
        "task36_cpp_test_4.txt",
        "task36_cpp_test_5.txt"
    };
    std::vector<std::string> contents = {
        "C++ Test File 1\nLine A\nLine B",
        "C++ Test File 2\nHello World",
        "C++ Test File 3\nSample Content",
        "C++ Test File 4\nLorem Ipsum",
        "C++ Test File 5\nEnd of File"
    };

    for (size_t i = 0; i < filenames.size(); ++i) {
        std::ofstream out(filenames[i], std::ios::out | std::ios::binary);
        out << contents[i];
        out.close();
    }

    for (size_t i = 0; i < filenames.size(); ++i) {
        std::cout << "== C++ Test " << (i + 1) << " ==\n";
        std::string data = readFile(filenames[i]);
        if (!data.empty()) {
            std::cout << data << "\n";
        } else {
            std::cout << "Error reading file: " << filenames[i] << "\n";
        }
        std::remove(filenames[i].c_str());
    }

    return 0;
}