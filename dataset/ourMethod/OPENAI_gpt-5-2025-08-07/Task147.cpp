#include <iostream>
#include <vector>
#include <queue>
#include <string>

struct KthResult {
    bool ok;
    int value;
    std::string error;
};

KthResult kth_largest(const std::vector<int>& arr, int k) {
    if (arr.empty()) {
        return {false, 0, "Invalid input: array is empty."};
    }
    if (k < 1 || static_cast<size_t>(k) > arr.size()) {
        return {false, 0, "Invalid input: k out of range."};
    }

    std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
    for (int v : arr) {
        if (static_cast<int>(minHeap.size()) < k) {
            minHeap.push(v);
        } else if (v > minHeap.top()) {
            minHeap.pop();
            minHeap.push(v);
        }
    }
    if (static_cast<int>(minHeap.size()) != k) {
        return {false, 0, "Unexpected error: heap size mismatch."};
    }
    return {true, minHeap.top(), ""};
}

void runTest(const std::vector<int>& arr, int k) {
    KthResult r = kth_largest(arr, k);
    if (r.ok) {
        std::cout << "k=" << k << ", result=" << r.value << "\n";
    } else {
        std::cout << "k=" << k << ", error=" << r.error << "\n";
    }
}

int main() {
    runTest({3, 2, 1, 5, 6, 4}, 2);             // expected 5
    runTest({7, 10, 4, 3, 20, 15}, 3);          // expected 10
    runTest({-1, -2, -3, -4}, 1);               // expected -1
    runTest({5, 5, 5, 5}, 2);                   // expected 5
    runTest({1, 23, 12, 9, 30, 2, 50}, 4);      // expected 12
    // Example invalid test (k out of range)
    runTest({1}, 2);                            // error
    return 0;
}