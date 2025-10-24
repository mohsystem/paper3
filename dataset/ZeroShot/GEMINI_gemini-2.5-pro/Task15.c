#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void freeTower(char** tower, int nFloors);

/**
 * @brief Builds a pyramid-shaped tower as a 2D char array.
 * 
 * @param nFloors The number of floors in the tower. Must be a positive integer.
 * @return A dynamically allocated array of strings. The caller is responsible for freeing
 *         the memory using the freeTower function. Returns NULL for nFloors <= 0 or on allocation failure.
 */
char** buildTower(int nFloors) {
    if (nFloors <= 0) {
        return NULL;
    }

    char** tower = (char**)malloc(nFloors * sizeof(char*));
    if (tower == NULL) {
        return NULL; // Allocation failure
    }

    int width = 2 * nFloors - 1;

    for (int i = 0; i < nFloors; i++) {
        tower[i] = (char*)malloc((width + 1) * sizeof(char));
        if (tower[i] == NULL) {
            // Allocation failure for a floor, clean up and exit
            freeTower(tower, i); // Free already allocated floors
            return NULL;
        }

        int numStars = 2 * i + 1;
        int numSpaces = (width - numStars) / 2;

        memset(tower[i], ' ', numSpaces);
        memset(tower[i] + numSpaces, '*', numStars);
        memset(tower[i] + numSpaces + numStars, ' ', numSpaces);
        tower[i][width] = '\0'; // Null-terminate the string
    }

    return tower;
}

/**
 * @brief Frees the memory allocated for the tower.
 */
void freeTower(char** tower, int nFloors) {
    if (tower == NULL || nFloors <= 0) {
        return;
    }
    for (int i = 0; i < nFloors; i++) {
        free(tower[i]);
    }
    free(tower);
}

/**
 * @brief Helper function to print a tower in the specified format.
 */
void printTower(char** tower, int nFloors) {
    if (tower == NULL || nFloors <= 0) {
        printf("[]\n");
        return;
    }
    printf("[\n");
    for (int i = 0; i < nFloors; i++) {
        printf("  \"%s\"%s\n", tower[i], (i == nFloors - 1) ? "" : ",");
    }
    printf("]\n");
}

int main() {
    int testCases[] = {1, 3, 6, 0, -2};
    int numTests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTests; ++i) {
        int n = testCases[i];
        printf("Tower with %d floors:\n", n);
        char** tower = buildTower(n);
        printTower(tower, n);
        freeTower(tower, n);
        printf("\n");
    }

    return 0;
}