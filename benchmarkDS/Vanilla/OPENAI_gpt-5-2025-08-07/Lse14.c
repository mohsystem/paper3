#include <stdio.h>

int getValueAtIndex(const int* id_sequence, int size, int index) {
    if (index >= 0 && index < size) {
        return id_sequence[index];
    }
    return -1; // invalid index
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70};
    int size = (int)(sizeof(id_sequence) / sizeof(id_sequence[0]));

    int userIndex;
    printf("Enter index: ");
    if (scanf("%d", &userIndex) != 1) {
        userIndex = -1;
    }

    int testIndices[5] = { userIndex, 0, 3, 6, -1 };

    for (int i = 0; i < 5; ++i) {
        printf("%d\n", getValueAtIndex(id_sequence, size, testIndices[i]));
    }

    return 0;
}