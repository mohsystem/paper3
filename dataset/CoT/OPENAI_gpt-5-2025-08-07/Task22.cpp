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

std::string vecToString(const std::vector<int>& v) {
    std::string s = "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s += ", ";
        s += std::to_string(v[i]);
    }
    s += "]";
    return s;
}

int main() {
    std::vector<Item> t1{1, 2, std::string("a"), std::string("b")};
    std::vector<Item> t2{1, std::string("a"), std::string("b"), 0, 15};
    std::vector<Item> t3{1, 2, std::string("a"), std::string("b"), std::string("aasf"), std::string("1"), std::string("123"), 123};
    std::vector<Item> t4{0, std::string("zero"), 99, std::string("99"), 100, -5};
    std::vector<Item> t5{std::string("only"), std::string("strings"), std::string("here")};

    std::vector<std::vector<Item>> tests{t1, t2, t3, t4, t5};

    for (size_t i = 0; i < tests.size(); ++i) {
        auto res = filterList(tests[i]);
        std::cout << "Test " << (i + 1) << ": " << vecToString(res) << "\n";
    }
    return 0;
}