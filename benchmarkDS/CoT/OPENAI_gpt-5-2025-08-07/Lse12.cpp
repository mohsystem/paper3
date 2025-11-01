#include <iostream>
#include <limits>

int getValueFromArray(const int* arr, int size, int index) {
    if (arr == nullptr || size <= 0) {
        return 0;
    }
    if (index >= 0 && index < size) {
        return arr[index];
    }
    return 0;
}

void runTests() {
    int arr[5] = {10, 20, 30, 40, 50};
    int size = 5;
    int tests[5] = {0, 2, 4, -1, 10};
    for (int i = 0; i < 5; ++i) {
        int res = getValueFromArray(arr, size, tests[i]);
        std::cout << "Test " << (i + 1) << ": index=" << tests[i] << " value=" << res << "\n";
    }
}

int main() {
    runTests();

    int arr[5] = {10, 20, 30, 40, 50};
    int size = 5;
    int index = 0;
    if (!(std::cin >> index)) {
        index = 0;
    }
    int value = getValueFromArray(arr, size, index);
    return value;
}