#include <iostream>
#include <vector>
#include <string>
#include <limits>

/**
 * @brief Builds a pyramid-shaped tower as a vector of strings.
 *
 * @param n_floors The number of floors in the tower. Must be a positive integer.
 * @return A std::vector<std::string> representing the tower. Returns an empty vector for invalid input.
 */
std::vector<std::string> build_tower(int n_floors) {
    // Rule #4: Input validation. The number of floors must be positive.
    if (n_floors <= 0) {
        return {};
    }
    
    // Rule #7: Integer overflow check. The width is 2 * n_floors - 1.
    // Check if 2 * n_floors would overflow before calculation.
    if (n_floors > std::numeric_limits<int>::max() / 2) {
        return {};
    }

    std::vector<std::string> tower;
    // Rule #7: Using std::vector and RAII for automatic memory management.
    tower.reserve(n_floors);

    int width = 2 * n_floors - 1;

    for (int i = 0; i < n_floors; ++i) {
        int num_stars = 2 * i + 1;
        int num_spaces = (width - num_stars) / 2;
        
        // Rule #7: Using std::string for safe string manipulation.
        // This construction avoids buffer overflows.
        std::string floor = std::string(num_spaces, ' ') +
                            std::string(num_stars, '*') +
                            std::string(num_spaces, ' ');
        tower.push_back(floor);
    }

    return tower;
}

/**
 * @brief Helper function to print a tower for verification.
 *
 * @param tower The tower to print, represented as a vector of strings.
 */
void print_tower(const std::vector<std::string>& tower) {
    if (tower.empty()) {
        std::cout << "[]" << std::endl;
        return;
    }
    std::cout << "[" << std::endl;
    for (size_t i = 0; i < tower.size(); ++i) {
        std::cout << "  \"" << tower[i] << "\"";
        if (i < tower.size() - 1) {
            std::cout << ",";
        }
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

/**
 * @brief Main function with test cases.
 */
int main() {
    std::cout << "--- Test Case 1: 3 floors ---" << std::endl;
    std::vector<std::string> tower1 = build_tower(3);
    print_tower(tower1);

    std::cout << "\n--- Test Case 2: 6 floors ---" << std::endl;
    std::vector<std::string> tower2 = build_tower(6);
    print_tower(tower2);

    std::cout << "\n--- Test Case 3: 1 floor ---" << std::endl;
    std::vector<std::string> tower3 = build_tower(1);
    print_tower(tower3);

    std::cout << "\n--- Test Case 4: 0 floors (invalid) ---" << std::endl;
    std::vector<std::string> tower4 = build_tower(0);
    print_tower(tower4);

    std::cout << "\n--- Test Case 5: -5 floors (invalid) ---" << std::endl;
    std::vector<std::string> tower5 = build_tower(-5);
    print_tower(tower5);

    return 0;
}