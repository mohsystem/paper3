#include <iostream>
#include <vector>
#include <stdexcept>

static const size_t MAX_LEN = 1000000;

static void siftDown(std::vector<int>& arr, size_t start, size_t heapSize) {
    size_t root = start;
    while (true) {
        size_t left = 2 * root + 1;
        if (left >= heapSize) break;
        size_t right = left + 1;
        size_t largest = root;
        if (arr[left] > arr[largest]) largest = left;
        if (right < heapSize && arr[right] > arr[largest]) largest = right;
        if (largest == root) break;
        std::swap(arr[root], arr[largest]);
        root = largest;
    }
}

std::vector<int> heapSort(const std::vector<int>& input) {
    if (input.size() > MAX_LEN) {
        throw std::invalid_argument("Invalid input length.");
    }
    std::vector<int> arr = input;
    size_t n = arr.size();

    // Build max heap
    if (n > 1) {
        for (size_t i = n / 2; i-- > 0;) {
            siftDown(arr, i, n);
        }
    }

    // Extract elements
    for (size_t end = n; end > 1; --end) {
        std::swap(arr[0], arr[end - 1]);
        siftDown(arr, 0, end - 1);
    }
    return arr;
}

static void printArray(const std::vector<int>& a) {
    std::cout << "[";
    for (size_t i = 0; i < a.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << a[i];
    }
    std::cout << "]\n";
}

int main() {
    try {
        std::vector<std::vector<int>> tests = {
            {3, 1, 4, 1, 5, 9, 2, 6, 5},
            {},
            {42},
            {1, 2, 3, 4, 5},
            {0, -1, -3, 2, -1, 2, 0}
        };
        for (const auto& t : tests) {
            auto sorted = heapSort(t);
            printArray(sorted);
        }
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
    return 0;
}