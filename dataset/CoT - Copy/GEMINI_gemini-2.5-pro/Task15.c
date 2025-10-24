#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to free the allocated memory for the tower
void freeTower(char** tower, int n_floors) {
    if (tower == NULL) {
        return;
    }
    for (int i = 0; i < n_floors; i++) {
        free(tower[i]); // Free each string
    }
    free(tower); // Free the array of pointers
}

/**
 * @brief Builds a pyramid-shaped tower as a dynamically allocated array of strings.
 * 
 * @param n_floors The number of floors in the tower (must be a positive integer).
 * @return A char** representing the tower. Returns NULL if n_floors is not positive or if memory allocation fails.
 *         The caller is responsible for freeing the memory using the freeTower function.
 */
char** buildTower(int n_floors) {
    if (n_floors <= 0) {
        return NULL;
    }

    // Total width of the tower's base
    int width = 2 * n_floors - 1;

    // Allocate memory for the array of string pointers
    char** tower = (char**)malloc(n_floors * sizeof(char*));
    if (tower == NULL) {
        return NULL; // Allocation failed
    }

    for (int i = 0; i < n_floors; i++) {
        // Allocate memory for the current floor string (+1 for null terminator)
        tower[i] = (char*)malloc((width + 1) * sizeof(char));
        if (tower[i] == NULL) {
            // Allocation failed, clean up previously allocated memory to prevent leaks
            freeTower(tower, i);
            return NULL;
        }

        int num_stars = 2 * i + 1;
        int num_spaces = (width - num_stars) / 2;

        // Fill the string with spaces and stars
        memset(tower[i], ' ', num_spaces);
        memset(tower[i] + num_spaces, '*', num_stars);
        memset(tower[i] + num_spaces + num_stars, ' ', num_spaces);
        
        // Add the null terminator
        tower[i][width] = '\0';
    }

    return tower;
}

int main() {
    int testCases[] = {1, 3, 6, 0, 5};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        int nFloors = testCases[i];
        printf("Test Case %d: nFloors = %d\n", i + 1, nFloors);
        
        char** tower = buildTower(nFloors);

        if (tower == NULL) {
            printf("[]\n");
        } else {
            printf("[\n");
            for (int j = 0; j < nFloors; j++) {
                printf("  \"%s\"\n", tower[j]);
            }
            printf("]\n");
            // Important: Free the allocated memory after use
            freeTower(tower, nFloors);
        }
        printf("--------------------\n");
    }

    return 0;
}