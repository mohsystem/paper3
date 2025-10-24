#include <iostream>
#include <vector>
#include <algorithm>

class Task143 {
public:
    static std::vector<int> sortArray(const std::vector<int>& arr) {
        std::vector<int> res = arr;
        std::sort(res.begin(), res.end());
        return res;
    }
};

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    std::vector<int> a1{3, 1, 2};
    std::vector<int> a2{5, -1, 4, 4, 2};
    std::vector<int> a3{};
    std::vector<int> a4{10};
    std::vector<int> a5{9, 8, 7, 6, 5, 4, 3, 2, 1};

    printVec(Task143::sortArray(a1));
    printVec(Task143::sortArray(a2));
    printVec(Task143::sortArray(a3));
    printVec(Task143::sortArray(a4));
    printVec(Task143::sortArray(a5));

    return 0;
}