#include <iostream>
#include <string>
#include <vector>

/**
 * Creates the display text for people who liked an item.
 *
 * @param names A constant reference to a vector of strings of people who liked the item.
 * @return The formatted string.
 */
std::string whoLikesIt(const std::vector<std::string>& names) {
    // Using std::string for concatenation is safe and handles memory management automatically.
    // This avoids buffer overflow vulnerabilities common with C-style strings.
    switch (names.size()) {
        case 0:
            return "no one likes this";
        case 1:
            return names[0] + " likes this";
        case 2:
            return names[0] + " and " + names[1] + " like this";
        case 3:
            return names[0] + ", " + names[1] + " and " + names[2] + " like this";
        default:
            // std::to_string is used for safe integer-to-string conversion.
            return names[0] + ", " + names[1] + " and " + std::to_string(names.size() - 2) + " others like this";
    }
}

int main() {
    // Test Cases
    std::cout << "Test Case 1: []" << std::endl;
    std::cout << "Expected: no one likes this" << std::endl;
    std::cout << "Actual:   " << whoLikesIt({}) << std::endl;
    std::cout << "--------------------" << std::endl;

    std::cout << "Test Case 2: [\"Peter\"]" << std::endl;
    std::cout << "Expected: Peter likes this" << std::endl;
    std::cout << "Actual:   " << whoLikesIt({"Peter"}) << std::endl;
    std::cout << "--------------------" << std::endl;

    std::cout << "Test Case 3: [\"Jacob\", \"Alex\"]" << std::endl;
    std::cout << "Expected: Jacob and Alex like this" << std::endl;
    std::cout << "Actual:   " << whoLikesIt({"Jacob", "Alex"}) << std::endl;
    std::cout << "--------------------" << std::endl;

    std::cout << "Test Case 4: [\"Max\", \"John\", \"Mark\"]" << std::endl;
    std::cout << "Expected: Max, John and Mark like this" << std::endl;
    std::cout << "Actual:   " << whoLikesIt({"Max", "John", "Mark"}) << std::endl;
    std::cout << "--------------------" << std::endl;

    std::cout << "Test Case 5: [\"Alex\", \"Jacob\", \"Mark\", \"Max\"]" << std::endl;
    std::cout << "Expected: Alex, Jacob and 2 others like this" << std::endl;
    std::cout << "Actual:   " << whoLikesIt({"Alex", "Jacob", "Mark", "Max"}) << std::endl;
    std::cout << "--------------------" << std::endl;

    return 0;
}