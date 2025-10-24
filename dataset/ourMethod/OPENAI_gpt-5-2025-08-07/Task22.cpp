#include <iostream>
#include <string>
#include <variant>
#include <vector>

using Elem = std::variant<long long, std::string>;

std::vector<long long> filterList(const std::vector<Elem>& input) {
    std::vector<long long> result;
    result.reserve(input.size());
    for (const auto& v : input) {
        if (const auto p = std::get_if<long long>(&v)) {
            if (*p >= 0) {
                result.push_back(*p);
            }
        }
    }
    return result;
}

static void printVec(const std::vector<long long>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    // Test case 1
    std::vector<Elem> t1 = {1LL, 2LL, std::string("a"), std::string("b")};
    auto r1 = filterList(t1);
    printVec(r1); // expected: [1, 2]

    // Test case 2
    std::vector<Elem> t2 = {1LL, std::string("a"), std::string("b"), 0LL, 15LL};
    auto r2 = filterList(t2);
    printVec(r2); // expected: [1, 0, 15]

    // Test case 3
    std::vector<Elem> t3 = {1LL, 2LL, std::string("a"), std::string("b"), std::string("aasf"), std::string("1"), std::string("123"), 123LL};
    auto r3 = filterList(t3);
    printVec(r3); // expected: [1, 2, 123]

    // Test case 4 - only strings
    std::vector<Elem> t4 = {std::string("x"), std::string("y"), std::string("z")};
    auto r4 = filterList(t4);
    printVec(r4); // expected: []

    // Test case 5 - includes a negative integer (validated out), and mixed entries
    std::vector<Elem> t5 = {0LL, 5LL, -3LL, std::string("x"), 42LL, std::string("0")};
    auto r5 = filterList(t5);
    printVec(r5); // expected: [0, 5, 42]

    return 0;
}