#include <stdio.h>

int getValueFromArray(const int* arr, int size, int index) {
    if (index >= 0 && index < size) {
        return arr[index];
    }
    return 0;
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = (int)(sizeof(id_sequence) / sizeof(id_sequence[0]));

    // 5 test cases
    int testIdx[5] = {0, 4, 9, -1, 100};
    for (int i = 0; i < 5; ++i) {
        int v = getValueFromArray(id_sequence, size, testIdx[i]);
        printf("test%d: idx=%d, val=%d\n", i + 1, testIdx[i], v);
    }

    int idx = 0;
    if (scanf("%d", &idx) != 1) {
        idx = 0;
    }
    int res = getValueFromArray(id_sequence, size, idx);
    return res;
}