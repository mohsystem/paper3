#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Generates a display text for who likes an item.
 *
 * @param names A constant reference to a vector of strings containing the names of people.
 * @return A string with the formatted display text.
 */
std::string likes(const std::vector<std::string>& names) {
    size_t n = names.size();
    switch (n) {
        case 0:
            return "no one likes this";
        case 1:
            return names[0] + " likes this";
        case 2:
            return names[0] + " and " + names[1] + " like this";
        case 3:
            return names[0] + ", " + names[1] + " and " + names[2] + " like this";
        default:
            return names[0] + ", " + names[1] + " and " + std::to_string(n - 2) + " others like this";
    }
}

/**
 * @brief Helper function to run a test case and print the result.
 * 
 * @param names The vector of names for the test case.
 * @param label A string label to describe the test case input.
 */
void run_test_case(const std::vector<std::string>& names, const std::string& label) {
    std::cout << label << " -> \"" << likes(names) << "\"" << std::endl;
}

int main() {
    // Test case 1: 0 names
    run_test_case({}, "[]");

    // Test case 2: 1 name
    run_test_case({"Peter"}, "[\"Peter\"]");

    // Test case 3: 2 names
    run_test_case({"Jacob", "Alex"}, "[\"Jacob\", \"Alex\"]");

    // Test case 4: 3 names
    run_test_case({"Max", "John", "Mark"}, "[\"Max\", \"John\", \"Mark\"]");

    // Test case 5: 4+ names
    run_test_case({"Alex", "Jacob", "Mark", "Max"}, "[\"Alex\", \"Jacob\", \"Mark\", \"Max\"]");

    return 0;
}