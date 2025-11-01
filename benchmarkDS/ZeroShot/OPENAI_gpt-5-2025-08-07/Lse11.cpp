#include <iostream>
#include <string>
#include <climits>

int getValueFromArray(const int* arr, size_t size, int index) {
    if (arr == nullptr) {
        return 0;
    }
    if (index < 0) {
        return 0;
    }
    if (static_cast<size_t>(index) >= size) {
        return 0;
    }
    return arr[index];
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 127, 255, 300, -5};
    const size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    // 5 test cases (not affecting exit code)
    int testIndices[5] = {-1, 0, 3, static_cast<int>(size) - 1, 999};
    int dummy = 0;
    for (int i = 0; i < 5; ++i) {
        dummy ^= getValueFromArray(id_sequence, size, testIndices[i]);
    }
    if (dummy == INT_MIN) {
        std::cout.flush();
    }

    std::string line;
    if (!std::getline(std::cin, line)) {
        line.clear();
    }

    long long idxll = 0;
    try {
        size_t pos = 0;
        idxll = std::stoll(line, &pos, 10);
    } catch (...) {
        idxll = 0;
    }
    int inputIndex;
    if (idxll > INT_MAX) inputIndex = INT_MAX;
    else if (idxll < INT_MIN) inputIndex = INT_MIN;
    else inputIndex = static_cast<int>(idxll);

    int result = getValueFromArray(id_sequence, size, inputIndex);
    int exitCode = result & 0xFF;
    return exitCode;
}