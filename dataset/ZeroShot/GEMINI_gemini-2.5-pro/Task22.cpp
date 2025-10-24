#include <iostream>
#include <vector>
#include <string>
#include <any>

/**
 * Filters a vector of std::any, returning a new vector containing only non-negative integers.
 *
 * @param list The input vector of std::any.
 * @return A new vector of integers.
 */
std::vector<int> filter_list(const std::vector<std::any>& list) {
    std::vector<int> filtered_list;
    for (const auto& item : list) {
        // Check if the type is int
        if (item.type() == typeid(int)) {
            try {
                int value = std::any_cast<int>(item);
                if (value >= 0) {
                    filtered_list.push_back(value);
                }
            } catch (const std::bad_any_cast& e) {
                // This block should ideally not be reached due to the typeid check,
                // but it's good practice for safety.
            }
        }
    }
    return filtered_list;
}

void print_vector(const std::string& prefix, const std::vector<int>& vec) {
    std::cout << prefix;
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1
    std::vector<std::any> list1 = {1, 2, std::string("a"), std::string("b")};
    std::cout << "Test 1 Input: [1, 2, \"a\", \"b\"]" << std::endl;
    print_vector("Test 1 Output: ", filter_list(list1));

    // Test Case 2
    std::vector<std::any> list2 = {1, std::string("a"), std::string("b"), 0, 15};
    std::cout << "Test 2 Input: [1, \"a\", \"b\", 0, 15]" << std::endl;
    print_vector("Test 2 Output: ", filter_list(list2));

    // Test Case 3
    std::vector<std::any> list3 = {1, 2, std::string("aasf"), std::string("1"), std::string("123"), 123};
    std::cout << "Test 3 Input: [1, 2, \"aasf\", \"1\", \"123\", 123]" << std::endl;
    print_vector("Test 3 Output: ", filter_list(list3));

    // Test Case 4: With negative numbers
    std::vector<std::any> list4 = {-1, std::string("c"), 5, -9, std::string("d")};
    std::cout << "Test 4 Input: [-1, \"c\", 5, -9, \"d\"]" << std::endl;
    print_vector("Test 4 Output: ", filter_list(list4));

    // Test Case 5: Empty list
    std::vector<std::any> list5 = {};
    std::cout << "Test 5 Input: []" << std::endl;
    print_vector("Test 5 Output: ", filter_list(list5));

    return 0;
}