#include <iostream>

int getValueFromArray(const int* arr, int size, int index) {
    if (index >= 0 && index < size) {
        return arr[index];
    }
    return 0;
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = static_cast<int>(sizeof(id_sequence) / sizeof(id_sequence[0]));

    // 5 test cases
    int testIdx[] = {0, 4, 9, -1, 100};
    for (int i = 0; i < 5; ++i) {
        int v = getValueFromArray(id_sequence, size, testIdx[i]);
        std::cout << "test" << (i + 1) << ": idx=" << testIdx[i] << ", val=" << v << std::endl;
    }

    int idx = 0;
    if (!(std::cin >> idx)) {
        idx = 0;
    }
    int res = getValueFromArray(id_sequence, size, idx);
    return res;
}