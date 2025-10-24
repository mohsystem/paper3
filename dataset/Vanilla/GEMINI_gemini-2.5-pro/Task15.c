#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Frees the memory allocated for the tower.
 *
 * @param tower The tower to be freed.
 * @param nFloors The number of floors in the tower.
 */
void freeTower(char** tower, int nFloors) {
    if (tower == NULL) {
        return;
    }
    for (int i = 0; i < nFloors; ++i) {
        free(tower[i]);
    }
    free(tower);
}

/**
 * Builds a pyramid-shaped tower as a dynamically allocated array of strings.
 *
 * @param nFloors The number of floors in the tower (must be positive).
 * @return A dynamically allocated array of C-style strings (char**).
 *         The caller is responsible for freeing the memory using freeTower().
 *         Returns a pointer from malloc(0) for nFloors <= 0.
 *         Returns NULL if memory allocation fails for nFloors > 0.
 */
char** buildTower(int nFloors) {
    if (nFloors <= 0) {
        // Return a valid pointer for a zero-sized allocation that can be safely freed.
        return (char**)malloc(0);
    }

    char** tower = (char**)malloc(nFloors * sizeof(char*));
    if (tower == NULL) {
        return NULL; // Memory allocation failed
    }

    int towerWidth = 2 * nFloors - 1;

    for (int i = 0; i < nFloors; ++i) {
        int floorNumber = i + 1;
        int stars = 2 * floorNumber - 1;
        int spaces = (towerWidth - stars) / 2;

        tower[i] = (char*)malloc((towerWidth + 1) * sizeof(char));
        if (tower[i] == NULL) {
            // Cleanup previously allocated memory on failure
            freeTower(tower, i);
            return NULL;
        }

        memset(tower[i], ' ', spaces);
        memset(tower[i] + spaces, '*', stars);
        memset(tower[i] + spaces + stars, ' ', spaces);
        tower[i][towerWidth] = '\0'; // Null-terminate the string
    }

    return tower;
}

int main() {
    int testCases[] = {1, 3, 6, 0, 10};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int n = testCases[i];
        printf("Test Case %d: nFloors = %d\n", i + 1, n);
        
        char** tower = buildTower(n);
        
        if (tower == NULL && n > 0) {
             printf("Memory allocation failed.\n");
        } else {
            printf("[\n");
            for (int j = 0; j < n; ++j) {
                printf("  \"%s\"\n", tower[j]);
            }
            printf("]\n");
        }
        
        // Always call freeTower, it handles NULL and n=0 correctly.
        freeTower(tower, n);
        
        printf("\n");
    }

    return 0;
}