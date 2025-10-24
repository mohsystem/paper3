#include <iostream>
#include <vector>
#include <string>
#include <limits>

std::vector<std::string> buildTower(int n) {
    std::vector<std::string> res;
    if (n <= 0) return res;
    const unsigned long long nULL = static_cast<unsigned long long>(n);
    const unsigned long long widthULL = 2ULL * nULL - 1ULL;
    if (widthULL > static_cast<unsigned long long>(std::numeric_limits<size_t>::max())) {
        return res;
    }
    const size_t width = static_cast<size_t>(widthULL);
    res.reserve(static_cast<size_t>(n));
    for (int i = 1; i <= n; ++i) {
        size_t stars = 2ULL * static_cast<unsigned long long>(i) - 1ULL;
        size_t spaces = (width - stars) / 2ULL;
        std::string line(width, ' ');
        for (size_t j = 0; j < stars; ++j) {
            line[spaces + j] = '*';
        }
        res.emplace_back(std::move(line));
    }
    return res;
}

static void printTower(const std::vector<std::string>& tower) {
    std::cout << "[\n";
    for (size_t i = 0; i < tower.size(); ++i) {
        std::cout << "  \"" << tower[i] << "\"" << (i + 1 < tower.size() ? "," : "") << "\n";
    }
    std::cout << "]\n";
}

int main() {
    std::vector<int> tests = {1, 2, 3, 5, 6};
    for (int t : tests) {
        std::cout << "Floors: " << t << "\n";
        auto tower = buildTower(t);
        printTower(tower);
        std::cout << "\n";
    }
    return 0;
}