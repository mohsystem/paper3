#include <iostream>
#include <vector>
#include <string>

class Task25 {
public:
    static std::vector<std::string> number(const std::vector<std::string>& lines) {
        std::vector<std::string> result;
        result.reserve(lines.size());
        unsigned long long idx = 1ULL;
        for (const auto& s : lines) {
            result.push_back(std::to_string(idx) + ": " + s);
            if (idx == std::numeric_limits<unsigned long long>::max()) {
                break;
            }
            ++idx;
        }
        return result;
    }
};

static void printVec(const std::vector<std::string>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << "\"" << v[i] << "\"";
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    // Test case 1: empty list
    printVec(Task25::number({}));
    // Test case 2: ["a", "b", "c"]
    printVec(Task25::number({"a", "b", "c"}));
    // Test case 3: ["", "x", ""]
    printVec(Task25::number({"", "x", ""}));
    // Test case 4: ["only one"]
    printVec(Task25::number({"only one"}));
    // Test case 5: multiple lines
    printVec(Task25::number({"line 0", "line 1", "line 2", "line 3", "line 4"}));
    return 0;
}