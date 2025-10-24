#include <iostream>
#include <vector>
#include <string>

/**
 * Builds a pyramid-shaped tower as a vector of strings.
 *
 * @param nFloors The number of floors in the tower (must be positive).
 * @return A std::vector<std::string> representing the tower.
 */
std::vector<std::string> buildTower(int nFloors) {
    if (nFloors <= 0) {
        return {};
    }

    std::vector<std::string> tower;
    tower.reserve(nFloors);
    int towerWidth = 2 * nFloors - 1;

    for (int i = 1; i <= nFloors; ++i) {
        int stars = 2 * i - 1;
        int spaces = (towerWidth - stars) / 2;
        tower.push_back(std::string(spaces, ' ') + std::string(stars, '*') + std::string(spaces, ' '));
    }
    return tower;
}

int main() {
    int testCases[] = {1, 3, 6, 0, 10};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int n = testCases[i];
        std::cout << "Test Case " << (i + 1) << ": nFloors = " << n << std::endl;
        std::vector<std::string> tower = buildTower(n);

        if (tower.empty()) {
            std::cout << "[]" << std::endl;
        } else {
            std::cout << "[" << std::endl;
            for (const auto& floor : tower) {
                std::cout << "  \"" << floor << "\"" << std::endl;
            }
            std::cout << "]" << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}