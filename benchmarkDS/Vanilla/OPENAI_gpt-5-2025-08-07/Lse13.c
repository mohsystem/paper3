#include <stdio.h>

static const int id_sequence[] = {101, 203, 305, 407, 509, 611, 713, 815, 917, 1020};
static const int ID_SIZE = sizeof(id_sequence) / sizeof(id_sequence[0]);

int getValueAtIndex(int index) {
    if (index >= 0 && index < ID_SIZE) {
        return id_sequence[index];
    }
    return -1; // Return -1 for invalid index
}

int main() {
    // 5 test cases
    int tests[5] = {0, 1, 4, 9, 10};
    int i;
    printf("Running 5 test cases:\n");
    for (i = 0; i < 5; ++i) {
        int idx = tests[i];
        printf("Index %d -> %d\n", idx, getValueAtIndex(idx));
    }

    // Prompt user for input
    int idx;
    printf("Enter index: ");
    if (scanf("%d", &idx) == 1) {
        printf("%d\n", getValueAtIndex(idx));
    } else {
        printf("Invalid input\n");
    }
    return 0;
}