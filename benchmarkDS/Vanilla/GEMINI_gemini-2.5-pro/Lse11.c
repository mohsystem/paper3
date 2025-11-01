#include <stdio.h>
#include <assert.h>

int getValueFromArray(int arr[], int size, int index) {
    return arr[index];
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // 5 Test cases using assertions
    assert(getValueFromArray(id_sequence, size, 0) == 10);
    assert(getValueFromArray(id_sequence, size, 1) == 20);
    assert(getValueFromArray(id_sequence, size, 2) == 30);
    assert(getValueFromArray(id_sequence, size, 3) == 40);
    assert(getValueFromArray(id_sequence, size, 4) == 50);

    int index;
    if (scanf("%d", &index) == 1) {
        if (index >= 0 && index < size) {
            int value = getValueFromArray(id_sequence, size, index);
            return value;
        } else {
            return 1; // Return an error code for out-of-bounds index
        }
    }
    return 1; // Return an error code for non-integer input
}