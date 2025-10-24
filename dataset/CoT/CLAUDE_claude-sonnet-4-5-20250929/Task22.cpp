
#include <iostream>
#include <vector>
#include <string>
#include <variant>

std::vector<int> filterList(const std::vector<std::variant<int, std::string>>& list) {
    std::vector<int> result;
    
    for (const auto& item : list) {
        if (std::holds_alternative<int>(item)) {
            result.push_back(std::get<int>(item));
        }
    }
    
    return result;
}

void printVector(const std::vector<int>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test case 1
    std::vector<std::variant<int, std::string>> test1 = {1, 2, std::string("a"), std::string("b")};
    printVector(filterList(test1));
    
    // Test case 2
    std::vector<std::variant<int, std::string>> test2 = {1, std::string("a"), std::string("b"), 0, 15};
    printVector(filterList(test2));
    
    // Test case 3
    std::vector<std::variant<int, std::string>> test3 = {1, 2, std::string("a"), std::string("b"), std::string("aasf"), std::string("1"), std::string("123"), 123};
    printVector(filterList(test3));
    
    // Test case 4
    std::vector<std::variant<int, std::string>> test4 = {};
    printVector(filterList(test4));
    
    // Test case 5
    std::vector<std::variant<int, std::string>> test5 = {100, 200, std::string("test"), 300};
    printVector(filterList(test5));
    
    return 0;
}
