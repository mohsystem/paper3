#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** build_tower(int n, int* outSize) {
    if (outSize) *outSize = 0;
    if (n <= 0) return NULL;
    int width = 2 * n - 1;
    char** rows = (char**)malloc(sizeof(char*) * n);
    if (!rows) return NULL;
    for (int i = 1; i <= n; ++i) {
        int stars = 2 * i - 1;
        int spaces = n - i;
        char* row = (char*)malloc((width + 1) * sizeof(char));
        if (!row) {
            for (int k = 0; k < i - 1; ++k) free(rows[k]);
            free(rows);
            return NULL;
        }
        int idx = 0;
        for (int s = 0; s < spaces; ++s) row[idx++] = ' ';
        for (int st = 0; st < stars; ++st) row[idx++] = '*';
        for (int s = 0; s < spaces; ++s) row[idx++] = ' ';
        row[idx] = '\0';
        rows[i - 1] = row;
    }
    if (outSize) *outSize = n;
    return rows;
}

void free_tower(char** tower, int size) {
    if (!tower) return;
    for (int i = 0; i < size; ++i) free(tower[i]);
    free(tower);
}

int main() {
    int tests[5] = {1, 2, 3, 4, 6};
    for (int ti = 0; ti < 5; ++ti) {
        int n = tests[ti];
        printf("Tower with %d floors:\n", n);
        int size = 0;
        char** tower = build_tower(n, &size);
        for (int i = 0; i < size; ++i) {
            printf("%s\n", tower[i]);
        }
        printf("\n");
        free_tower(tower, size);
    }
    return 0;
}