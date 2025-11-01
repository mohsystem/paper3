#include <stdio.h>
#include <limits.h>

int getValueAtIndex(const int* id_sequence, int length, int index) {
    if (index < 0 || index >= length) {
        return INT_MIN; // invalid index sentinel
    }
    return id_sequence[index];
}

int main(void) {
    int id_sequence[] = {11, 22, 33, 44, 55, 66, 77, 88, 99};
    int length = (int)(sizeof(id_sequence) / sizeof(id_sequence[0]));

    int idx;
    printf("Enter index: ");
    if (scanf("%d", &idx) == 1) {
        int val = getValueAtIndex(id_sequence, length, idx);
        if (val == INT_MIN) {
            printf("Invalid index: %d\n", idx);
        } else {
            printf("Value at index %d: %d\n", idx, val);
        }
    } else {
        printf("Invalid input.\n");
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }

    int testIndices[5] = {0, 3, length - 1, -1, 100};
    for (int i = 0; i < 5; ++i) {
        int ti = testIndices[i];
        int val = getValueAtIndex(id_sequence, length, ti);
        if (val == INT_MIN) {
            printf("Test %d -> index %d invalid\n", i + 1, ti);
        } else {
            printf("Test %d -> index %d value: %d\n", i + 1, ti, val);
        }
    }

    return 0;
}