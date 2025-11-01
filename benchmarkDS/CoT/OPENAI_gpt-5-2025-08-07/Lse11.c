/* Chain-of-Through secure code generation:
 * 1) Problem understanding: Provide getValueFromArray to safely access array values by index.
 * 2) Security requirements: Validate inputs, avoid out-of-bounds access, and normalize exit codes.
 * 3) Secure coding generation: Implement bounds checks and handle input parsing safely.
 * 4) Code review: Avoid undefined behavior and ensure consistent behavior across environments.
 * 5) Secure code output: Final code reflects mitigations and robust handling.
 */

#include <stdio.h>
#include <stdlib.h>

int getValueFromArray(const int* arr, size_t size, int index) {
    if (arr == NULL) {
        return 0;
    }
    if (index < 0 || (size_t)index >= size) {
        return 0;
    }
    return arr[index];
}

static void runTests(void) {
    const int id_sequence[] = {1, 12, 123, 200, 255};
    const size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    const int testIndices[5] = {0, 4, 2, -1, 10};

    for (int i = 0; i < 5; ++i) {
        int idx = testIndices[i];
        int val = getValueFromArray(id_sequence, size, idx);
        printf("Index: %d -> Value: %d\n", idx, val);
    }
}

int main(int argc, char* argv[]) {
    const int id_sequence[] = {1, 12, 123, 200, 255};
    const size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    if (argc > 1) {
        runTests();
        return 0;
    }

    int idx = 0;
    if (scanf("%d", &idx) != 1) {
        return 0;
    }

    int value = getValueFromArray(id_sequence, size, idx);
    return (value & 0xFF);
}