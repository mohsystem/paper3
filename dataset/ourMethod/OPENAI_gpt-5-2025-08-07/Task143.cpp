#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

static const std::size_t MAX_LEN = 1'000'000;

std::vector<int> sortAscending(const std::vector<int>& input) {
    if (input.size() > MAX_LEN) {
        throw std::invalid_argument("Input vector too large.");
    }
    std::vector<int> copy = input;
    std::sort(copy.begin(), copy.end());
    return copy;
}

void printVector(const std::vector<int>& v) {
    std::cout << "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]";
}

void runTest(int testNumber, const std::vector<int>& input) {
    try {
        std::vector<int> sorted = sortAscending(input);
        std::cout << "Test " << testNumber << " input:  ";
        printVector(input);
        std::cout << "\nTest " << testNumber << " output: ";
        printVector(sorted);
        std::cout << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test " << testNumber << " error: " << e.what() << "\n";
    }
    std::cout << "----\n";
}

int main() {
    std::vector<int> t1{3, 1, 2};
    std::vector<int> t2{-5, -10, 0, 5};
    std::vector<int> t3{};
    std::vector<int> t4{1, 1, 1, 1};
    std::vector<int> t5{2147483647, -2147483648, 0, -1, 1};

    runTest(1, t1);
    runTest(2, t2);
    runTest(3, t3);
    runTest(4, t4);
    runTest(5, t5);

    return 0;
}