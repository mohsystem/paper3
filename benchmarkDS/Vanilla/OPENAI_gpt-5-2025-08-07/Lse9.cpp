#include <iostream>
#include <vector>
#include <string>

std::string getArgAt(const std::vector<std::string>& argv, int index) {
    if (index >= 0 && index < static_cast<int>(argv.size())) return argv[index];
    return "";
}

int main(int argc, char* argv[]) {
    // 5 test cases
    std::vector<std::vector<std::string>> tests = {
        {"prog", "alpha", "beta", "gamma"},
        {"prog"},
        {"app", "x"},
        {"p", "10", "20", "30"},
        {"name", "first", "second"}
    };
    std::vector<int> indices = {1, 0, 5, 3, 2};
    for (int i = 0; i < 5; ++i) {
        std::string res = getArgAt(tests[i], indices[i]);
        if (!res.empty()) std::cout << res << "\n";
        else std::cout << "Index out of range\n";
    }

    // Handle actual command line
    if (argc >= 2) {
        try {
            int idx = std::stoi(argv[1]);
            std::vector<std::string> full;
            for (int i = 0; i < argc; ++i) full.emplace_back(argv[i]);
            std::string res = getArgAt(full, idx);
            if (!res.empty()) std::cout << res << "\n";
            else std::cout << "Index out of range\n";
        } catch (...) {
            std::cout << "Invalid index\n";
        }
    }
    return 0;
}