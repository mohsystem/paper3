#include <iostream>
#include <string>
#include <variant>
#include <vector>

using Item = std::variant<int, std::string>;

std::vector<int> filterList(const std::vector<Item>& items) {
    std::vector<int> result;
    result.reserve(items.size());
    for (const auto& it : items) {
        if (std::holds_alternative<int>(it)) {
            int v = std::get<int>(it);
            if (v >= 0) {
                result.push_back(v);
            }
        }
    }
    return result;
}

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    // Test case 1
    std::vector<Item> t1 = {1, 2, std::string("a"), std::string("b")};
    printVec(filterList(t1)); // [1, 2]

    // Test case 2
    std::vector<Item> t2 = {1, std::string("a"), std::string("b"), 0, 15};
    printVec(filterList(t2)); // [1, 0, 15]

    // Test case 3
    std::vector<Item> t3 = {1, 2, std::string("a"), std::string("b"), std::string("aasf"), std::string("1"), std::string("123"), 123};
    printVec(filterList(t3)); // [1, 2, 123]

    // Test case 4 (includes negative integer)
    std::vector<Item> t4 = {std::string("hello"), -1, 3, std::string("4"), 0};
    printVec(filterList(t4)); // [3, 0]

    // Test case 5 (empty list)
    std::vector<Item> t5 = {};
    printVec(filterList(t5)); // []
    return 0;
}