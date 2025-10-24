#include <iostream>
#include <vector>
#include <string>
#include <limits>

static const int MAX_FLOORS = 1000;

std::vector<std::string> buildTower(int floors) {
    std::vector<std::string> result;
    if (floors <= 0 || floors > MAX_FLOORS) {
        return result; // invalid input -> empty
    }

    const int width = 2 * floors - 1;
    result.reserve(static_cast<size_t>(floors));
    for (int i = 1; i <= floors; ++i) {
        int stars = 2 * i - 1;
        int spaces = (width - stars) / 2;

        if (spaces < 0 || stars < 0) { // defensive check
            result.clear();
            return result;
        }

        std::string line;
        line.reserve(static_cast<size_t>(width));
        line.append(static_cast<size_t>(spaces), ' ');
        line.append(static_cast<size_t>(stars), '*');
        line.append(static_cast<size_t>(spaces), ' ');
        result.push_back(line);
    }
    return result;
}

static void printTower(const std::vector<std::string>& tower) {
    if (tower.empty()) {
        std::cout << "(invalid or empty tower)" << std::endl;
        return;
    }
    for (const auto& line : tower) {
        std::cout << line << std::endl;
    }
}

int main() {
    // 5 test cases
    const int tests[5] = {1, 2, 3, 6, 0};
    for (int i = 0; i < 5; ++i) {
        int floors = tests[i];
        std::cout << "Floors = " << floors << std::endl;
        std::vector<std::string> tower = buildTower(floors);
        printTower(tower);
        std::cout << "----" << std::endl;
    }
    return 0;
}