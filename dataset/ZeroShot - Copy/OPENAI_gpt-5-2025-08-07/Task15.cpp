#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> build_tower(int n) {
    std::vector<std::string> result;
    if (n <= 0) return result;
    int width = 2 * n - 1;
    result.reserve(n);
    for (int i = 1; i <= n; ++i) {
        int stars = 2 * i - 1;
        int spaces = (width - stars) / 2;
        std::string line;
        line.reserve(width);
        line.append(spaces, ' ');
        line.append(stars, '*');
        line.append(spaces, ' ');
        result.push_back(std::move(line));
    }
    return result;
}

static void print_tower(const std::vector<std::string>& tower) {
    for (const auto& line : tower) {
        std::cout << line << '\n';
    }
}

int main() {
    std::vector<int> tests = {1, 2, 3, 6, 0};
    for (int n : tests) {
        std::cout << "n = " << n << "\n";
        print_tower(build_tower(n));
        std::cout << "\n";
    }
    return 0;
}