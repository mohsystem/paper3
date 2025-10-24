#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> number(const std::vector<std::string>& lines) {
    std::vector<std::string> result;
    result.reserve(lines.size());
    for (size_t i = 0; i < lines.size(); ++i) {
        result.emplace_back(std::to_string(i + 1) + ": " + lines[i]);
    }
    return result;
}

void printVec(const std::vector<std::string>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << "\"" << v[i] << "\"";
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    // Test case 1: Empty vector
    std::vector<std::string> t1;
    printVec(number(t1));

    // Test case 2: {"a", "b", "c"}
    std::vector<std::string> t2 = {"a", "b", "c"};
    printVec(number(t2));

    // Test case 3: {"", "foo", "bar"}
    std::vector<std::string> t3 = {"", "foo", "bar"};
    printVec(number(t3));

    // Test case 4: {"hello world", "Line 2"}
    std::vector<std::string> t4 = {"hello world", "Line 2"};
    printVec(number(t4));

    // Test case 5: {"multiple", "lines", "with", "numbers", "10"}
    std::vector<std::string> t5 = {"multiple", "lines", "with", "numbers", "10"};
    printVec(number(t5));

    return 0;
}