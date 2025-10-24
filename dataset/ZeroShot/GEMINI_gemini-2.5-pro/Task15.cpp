#include <iostream>
#include <vector>
#include <string>

/**
 * @brief Builds a pyramid-shaped tower as a vector of strings.
 * 
 * @param nFloors The number of floors in the tower. Must be a positive integer.
 * @return A vector of strings representing the tower. Returns an empty vector for nFloors <= 0.
 */
std::vector<std::string> buildTower(int nFloors) {
    if (nFloors <= 0) {
        return {}; // Return an empty vector
    }

    std::vector<std::string> tower;
    tower.reserve(nFloors); // Pre-allocate memory for efficiency

    for (int i = 0; i < nFloors; ++i) {
        int numStars = 2 * i + 1;
        int numSpaces = nFloors - 1 - i;
        
        tower.push_back(std::string(numSpaces, ' ') + std::string(numStars, '*') + std::string(numSpaces, ' '));
    }

    return tower;
}

/**
 * @brief Helper function to print a tower in the specified format.
 */
void printTower(const std::vector<std::string>& tower) {
    if (tower.empty()) {
        std::cout << "[]" << std::endl;
        return;
    }
    std::cout << "[" << std::endl;
    for (size_t i = 0; i < tower.size(); ++i) {
        std::cout << "  \"" << tower[i] << "\"" << (i == tower.size() - 1 ? "" : ",") << std::endl;
    }
    std::cout << "]" << std::endl;
}

int main() {
    int testCases[] = {1, 3, 6, 0, -2};
    int numTests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTests; ++i) {
        int n = testCases[i];
        std::cout << "Tower with " << n << " floors:" << std::endl;
        std::vector<std::string> tower = buildTower(n);
        printTower(tower);
        std::cout << std::endl;
    }

    return 0;
}