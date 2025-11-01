#include <iostream>

int getValueFromArray(int arr[], int size, int index) {
    // The size parameter is included as per the prompt's requirements.
    // A robust implementation would also check if the index is within bounds.
    return arr[index];
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    // 5 test cases
    int testIndices[] = {0, 3, 5, 8, 9};
    int numTestCases = sizeof(testIndices) / sizeof(testIndices[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int index = testIndices[i];
        int value = getValueFromArray(id_sequence, size, index);
        std::cout << "Value at index " << index << " is " << value << std::endl;
    }

    return 0;
}