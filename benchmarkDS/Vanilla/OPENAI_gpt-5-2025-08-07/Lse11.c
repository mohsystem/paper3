#include <stdio.h>

int getValueFromArray(const int* arr, int size, int index) {
    if (index < 0 || index >= size) return 0;
    return arr[index];
}

int main(void) {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80};
    int n = (int)(sizeof(id_sequence) / sizeof(id_sequence[0]));
    int idx;
    if (scanf("%d", &idx) == 1) {
        int val = getValueFromArray(id_sequence, n, idx);
        return val & 0xFF;
    } else {
        int tests[5] = {0, 3, n - 1, -1, n};
        int last = 0;
        for (int i = 0; i < 5; ++i) {
            int v = getValueFromArray(id_sequence, n, tests[i]);
            printf("%d\n", v);
            last = v;
        }
        return last & 0xFF;
    }
}