#include <iostream>
#include <vector>
#include <string>

/**
 * @brief Builds a pyramid-shaped tower as a vector of strings.
 * 
 * @param nFloors The number of floors in the tower (must be a positive integer).
 * @return A std::vector<std::string> representing the tower. Returns an empty vector if nFloors is not positive.
 */
std::vector<std::string> buildTower(int nFloors) {
    if (nFloors <= 0) {
        return {};
    }

    std::vector<std::string> tower;
    tower.reserve(nFloors); // Pre-allocate memory for efficiency

    for (int i = 1; i <= nFloors; ++i) {
        int num_spaces = nFloors - i;
        int num_stars = 2 * i - 1;
        
        // Construct the string for the current floor
        std::string floor = std::string(num_spaces, ' ') + 
                            std::string(num_stars, '*') + 
                            std::string(num_spaces, ' ');
        
        tower.push_back(floor);
    }
    
    return tower;
}

int main() {
    int testCases[] = {1, 3, 6, 0, 5};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int nFloors = testCases[i];
        std::cout << "Test Case " << (i + 1) << ": nFloors = " << nFloors << std::endl;
        std::vector<std::string> tower = buildTower(nFloors);
        
        if (tower.empty()) {
            std::cout << "[]" << std::endl;
        } else {
            std::cout << "[" << std::endl;
            for (const auto& floor : tower) {
                std::cout << "  \"" << floor << "\"" << std::endl;
            }
            std::cout << "]" << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
    
    return 0;
}