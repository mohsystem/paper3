#include <iostream>
#include <vector>
#include <string>
#include <any> // For std::any (C++17)

/**
 * @brief Filters a vector of std::any, returning a new vector with only the integers.
 * 
 * @param list The input vector containing non-negative integers and strings.
 * @return A new vector containing only the integers from the input vector.
 */
std::vector<int> filterList(const std::vector<std::any>& list) {
    std::vector<int> result;
    // Iterate through the input list.
    for (const auto& item : list) {
        // Safely check the type of the element in std::any.
        // Using item.type() is a secure way to avoid exceptions from bad casts.
        if (item.type() == typeid(int)) {
            // If the type is int, cast it and add to the result vector.
            result.push_back(std::any_cast<int>(item));
        }
    }
    return result;
}

// Helper function to print a vector of ints
void printIntVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

// Helper function to print a vector of std::any
void printAnyVector(const std::vector<std::any>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i].type() == typeid(int)) {
            std::cout << std::any_cast<int>(vec[i]);
        } else if (vec[i].type() == typeid(const char*)) {
            std::cout << "\"" << std::any_cast<const char*>(vec[i]) << "\"";
        }
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}


int main() {
    // Test Case 1
    std::vector<std::any> list1 = {1, 2, "a", "b"};
    std::cout << "Test Case 1: ";
    printAnyVector(list1);
    std::cout << "Filtered: ";
    printIntVector(filterList(list1)); // Expected: [1, 2]
    std::cout << std::endl;

    // Test Case 2
    std::vector<std::any> list2 = {1, "a", "b", 0, 15};
    std::cout << "Test Case 2: ";
    printAnyVector(list2);
    std::cout << "Filtered: ";
    printIntVector(filterList(list2)); // Expected: [1, 0, 15]
    std::cout << std::endl;

    // Test Case 3
    std::vector<std::any> list3 = {1, 2, "aasf", "1", "123", 123};
    std::cout << "Test Case 3: ";
    printAnyVector(list3);
    std::cout << "Filtered: ";
    printIntVector(filterList(list3)); // Expected: [1, 2, 123]
    std::cout << std::endl;

    // Test Case 4: All strings
    std::vector<std::any> list4 = {"hello", "world", "123"};
    std::cout << "Test Case 4: ";
    printAnyVector(list4);
    std::cout << "Filtered: ";
    printIntVector(filterList(list4)); // Expected: []
    std::cout << std::endl;

    // Test Case 5: Empty list
    std::vector<std::any> list5 = {};
    std::cout << "Test Case 5: ";
    printAnyVector(list5);
    std::cout << "Filtered: ";
    printIntVector(filterList(list5)); // Expected: []
    std::cout << std::endl;

    return 0;
}