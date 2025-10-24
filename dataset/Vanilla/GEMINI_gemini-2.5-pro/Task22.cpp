#include <iostream>
#include <vector>
#include <string>
#include <any>

/**
 * @brief Filters a vector of std::any, returning a new vector with only integers.
 * @param list The input vector containing integers and strings (or other types).
 * @return A new vector containing only the integers.
 */
std::vector<int> filter_list(const std::vector<std::any>& list) {
    std::vector<int> result;
    for (const auto& item : list) {
        if (item.type() == typeid(int)) {
            result.push_back(std::any_cast<int>(item));
        }
    }
    return result;
}

// Helper function to print a vector of std::any for testing
void print_any_vector(const std::vector<std::any>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i].type() == typeid(int)) {
            std::cout << std::any_cast<int>(vec[i]);
        } else if (vec[i].type() == typeid(const char*)) {
             std::cout << "\"" << std::any_cast<const char*>(vec[i]) << "\"";
        }
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

// Helper function to print a vector of ints
void print_int_vector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

int main() {
    // Test Case 1
    std::vector<std::any> list1 = {1, 2, "a", "b"};
    std::cout << "Test Case 1 Input: ";
    print_any_vector(list1);
    std::cout << std::endl;
    std::vector<int> filtered1 = filter_list(list1);
    std::cout << "Filtered Output: ";
    print_int_vector(filtered1);
    std::cout << std::endl << std::endl;

    // Test Case 2
    std::vector<std::any> list2 = {1, "a", "b", 0, 15};
    std::cout << "Test Case 2 Input: ";
    print_any_vector(list2);
    std::cout << std::endl;
    std::vector<int> filtered2 = filter_list(list2);
    std::cout << "Filtered Output: ";
    print_int_vector(filtered2);
    std::cout << std::endl << std::endl;

    // Test Case 3
    std::vector<std::any> list3 = {1, 2, "aasf", "1", "123", 123};
    std::cout << "Test Case 3 Input: ";
    print_any_vector(list3);
    std::cout << std::endl;
    std::vector<int> filtered3 = filter_list(list3);
    std::cout << "Filtered Output: ";
    print_int_vector(filtered3);
    std::cout << std::endl << std::endl;

    // Test Case 4
    std::vector<std::any> list4 = {"hello", "world", 2024};
    std::cout << "Test Case 4 Input: ";
    print_any_vector(list4);
    std::cout << std::endl;
    std::vector<int> filtered4 = filter_list(list4);
    std::cout << "Filtered Output: ";
    print_int_vector(filtered4);
    std::cout << std::endl << std::endl;

    // Test Case 5
    std::vector<std::any> list5 = {"a", "b", "c"};
    std::cout << "Test Case 5 Input: ";
    print_any_vector(list5);
    std::cout << std::endl;
    std::vector<int> filtered5 = filter_list(list5);
    std::cout << "Filtered Output: ";
    print_int_vector(filtered5);
    std::cout << std::endl << std::endl;
    
    return 0;
}