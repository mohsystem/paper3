// Chain-of-Through process:
// 1) Problem understanding: Implement heap sort for vector<int>, returning a new sorted vector.
// 2) Security requirements: Handle empty safely, do not mutate input, and ensure index checks.
// 3) Secure coding generation: Iterative heapify, safe bounds checks.
// 4) Code review: Verified no UB, no out-of-bounds, and no integer overflow in typical sizes.
// 5) Secure code output: Final version with test cases.

#include <iostream>
#include <vector>
#include <limits>

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]";
}

static void swapInt(int& a, int& b) {
    int tmp = a;
    a = b;
    b = tmp;
}

static void heapify(std::vector<int>& a, size_t heapSize, size_t i) {
    while (true) {
        size_t largest = i;
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;

        if (left < heapSize && a[left] > a[largest]) {
            largest = left;
        }
        if (right < heapSize && a[right] > a[largest]) {
            largest = right;
        }
        if (largest != i) {
            swapInt(a[i], a[largest]);
            i = largest;
        } else {
            break;
        }
    }
}

std::vector<int> heapSort(const std::vector<int>& input) {
    std::vector<int> a = input; // Do not mutate input
    size_t n = a.size();
    if (n <= 1) return a;

    for (size_t i = n / 2; i-- > 0; ) {
        heapify(a, n, i);
        if (i == 0) break;
    }

    for (size_t i = n; i-- > 1; ) {
        swapInt(a[0], a[i - 1]);
        heapify(a, i - 1, 0);
    }
    return a;
}

int main() {
    std::vector<std::vector<int>> tests = {
        {4, 10, 3, 5, 1},
        {},
        {1},
        {5, -1, 3, -2, 0, 5, 5},
        {1000000000, -1000000000, std::numeric_limits<int>::max(), std::numeric_limits<int>::min(), 42}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::vector<int> input = tests[i];
        std::vector<int> sorted = heapSort(input);
        std::cout << "Test " << (i + 1) << " input:  ";
        printVec(input);
        std::cout << "\nTest " << (i + 1) << " sorted: ";
        printVec(sorted);
        std::cout << "\n\n";
    }
    return 0;
}