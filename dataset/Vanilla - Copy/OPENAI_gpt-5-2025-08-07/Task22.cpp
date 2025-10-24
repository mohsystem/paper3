#include <iostream>
#include <vector>
#include <string>
#include <variant>

using Item = std::variant<int, std::string>;

std::vector<int> filter_list(const std::vector<Item>& items) {
    std::vector<int> result;
    for (const auto& it : items) {
        if (std::holds_alternative<int>(it)) {
            result.push_back(std::get<int>(it));
        }
    }
    return result;
}

void print_vec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    std::vector<Item> t1 = {1, 2, std::string("a"), std::string("b")};
    print_vec(filter_list(t1)); // [1, 2]

    std::vector<Item> t2 = {1, std::string("a"), std::string("b"), 0, 15};
    print_vec(filter_list(t2)); // [1, 0, 15]

    std::vector<Item> t3 = {1, 2, std::string("a"), std::string("b"), std::string("aasf"), std::string("1"), std::string("123"), 123};
    print_vec(filter_list(t3)); // [1, 2, 123]

    std::vector<Item> t4 = {std::string("x"), std::string("y"), std::string("z")};
    print_vec(filter_list(t4)); // []

    std::vector<Item> t5 = {0, 99, 1000, std::string("1000"), std::string(""), 5};
    print_vec(filter_list(t5)); // [0, 99, 1000, 5]

    return 0;
}