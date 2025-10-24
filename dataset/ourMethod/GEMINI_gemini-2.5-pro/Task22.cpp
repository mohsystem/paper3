#include <iostream>
#include <vector>
#include <string>
#include <variant>

// The variant type represents an element that can be either an int or a std::string.
using ListItem = std::variant<int, std::string>;

/**
 * @brief Filters a list of variants, returning only the integers.
 *
 * This function takes a vector of std::variant<int, std::string> and returns
 * a new vector containing only the integer elements.
 *
 * @param list A constant reference to the input vector of ListItems.
 * @return A std::vector<int> containing only the integer elements from the input list.
 */
std::vector<int> filterList(const std::vector<ListItem>& list) {
    std::vector<int> filtered_list;
    for (const auto& item : list) {
        // Check if the variant currently holds an int
        if (std::holds_alternative<int>(item)) {
            // Get the int value and add it to the result vector
            filtered_list.push_back(std::get<int>(item));
        }
    }
    return filtered_list;
}

// Helper function to print a vector of integers
void print_vector(const std::string& prefix, const std::vector<int>& vec) {
    std::cout << prefix;
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i == vec.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1
    std::vector<ListItem> list1 = {1, 2, "a", "b"};
    std::vector<int> result1 = filterList(list1);
    std::cout << "Test Case 1: [1, 2, \"a\", \"b\"] -> ";
    print_vector("[", result1);

    // Test Case 2
    std::vector<ListItem> list2 = {1, "a", "b", 0, 15};
    std::vector<int> result2 = filterList(list2);
    std::cout << "Test Case 2: [1, \"a\", \"b\", 0, 15] -> ";
    print_vector("[", result2);

    // Test Case 3
    std::vector<ListItem> list3 = {1, 2, "aasf", "1", "123", 123};
    std::vector<int> result3 = filterList(list3);
    std::cout << "Test Case 3: [1, 2, \"aasf\", \"1\", \"123\", 123] -> ";
    print_vector("[", result3);

    // Test Case 4: Empty list
    std::vector<ListItem> list4 = {};
    std::vector<int> result4 = filterList(list4);
    std::cout << "Test Case 4: [] -> ";
    print_vector("[", result4);

    // Test Case 5: List with only strings
    std::vector<ListItem> list5 = {"hello", "world", "!"};
    std::vector<int> result5 = filterList(list5);
    std::cout << "Test Case 5: [\"hello\", \"world\", \"!\"] -> ";
    print_vector("[", result5);

    return 0;
}