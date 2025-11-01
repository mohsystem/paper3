// Chain-of-Through secure code generation:
// 1) Problem understanding: Provide getValueFromArray to safely access array values by index.
// 2) Security requirements: Validate inputs, avoid out-of-bounds access, and normalize exit codes.
// 3) Secure coding generation: Implement bounds checks and handle input parsing safely.
// 4) Code review: Avoid undefined behavior and ensure consistent behavior across environments.
// 5) Secure code output: Final code reflects mitigations and robust handling.

#include <iostream>
#include <cstdlib>

int getValueFromArray(const int* arr, std::size_t size, int index) {
    if (arr == nullptr || static_cast<long long>(size) < 0) {
        return 0;
    }
    if (index < 0 || static_cast<std::size_t>(index) >= size) {
        return 0;
    }
    return arr[index];
}

static void runTests() {
    const int id_sequence[] = {1, 12, 123, 200, 255};
    const std::size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    const int testIndices[5] = {0, 4, 2, -1, 10};
    for (int idx : testIndices) {
        int val = getValueFromArray(id_sequence, size, idx);
        std::cout << "Index: " << idx << " -> Value: " << val << "\n";
    }
}

int main(int argc, char* argv[]) {
    const int id_sequence[] = {1, 12, 123, 200, 255};
    const std::size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    if (argc > 1) {
        runTests();
        return 0;
    }

    long long idxLL = 0;
    if (!(std::cin >> idxLL)) {
        return 0;
    }
    int idx = static_cast<int>(idxLL);
    int value = getValueFromArray(id_sequence, size, idx);
    return (value & 0xFF);
}