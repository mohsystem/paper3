#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Forward declaration
void free_tower(char** tower, int n_floors);

/**
 * @brief Builds a pyramid-shaped tower as a dynamically allocated array of strings.
 *
 * @param n_floors The number of floors in the tower. Must be a positive integer.
 * @param out_n_floors Pointer to an integer to store the number of floors in the returned array.
 * @return A dynamically allocated array of C-strings (char**). Returns NULL for invalid input or allocation failure.
 *         The caller is responsible for freeing the memory using free_tower().
 */
char** build_tower(int n_floors, int* out_n_floors) {
    // Rule #4: Input validation. n_floors must be a positive integer.
    if (n_floors <= 0) {
        *out_n_floors = 0;
        return NULL;
    }

    // C Security Checklist: Integer overflow check before arithmetic.
    // The width is 2 * n_floors - 1. Check if 2 * n_floors overflows.
    if (n_floors > INT_MAX / 2) {
        *out_n_floors = 0;
        return NULL;
    }
    
    // C Security Checklist: All malloc/calloc calls checked for NULL return.
    char** tower = (char**)malloc(n_floors * sizeof(char*));
    if (tower == NULL) {
        *out_n_floors = 0;
        return NULL;
    }

    int width = 2 * n_floors - 1;

    for (int i = 0; i < n_floors; ++i) {
        // C Security Checklist: Buffer sizes validated. Allocate for string + null terminator.
        tower[i] = (char*)malloc((width + 1) * sizeof(char));
        if (tower[i] == NULL) {
            // C Security Checklist: All allocated memory freed exactly once.
            // Cleanup previously allocated memory on failure.
            free_tower(tower, i);
            *out_n_floors = 0;
            return NULL;
        }

        int num_stars = 2 * i + 1;
        int num_spaces = (width - num_stars) / 2;

        // Rule #1 & #2: Ensure buffer operations adhere to boundaries.
        // memset is safe here because sizes are calculated correctly.
        memset(tower[i], ' ', num_spaces);
        memset(tower[i] + num_spaces, '*', num_stars);
        memset(tower[i] + num_spaces + num_stars, ' ', num_spaces);
        
        // C Security Checklist: All strings null-terminated.
        tower[i][width] = '\0';
    }

    *out_n_floors = n_floors;
    return tower;
}

/**
 * @brief Frees the memory allocated for a tower.
 *
 * @param tower The tower to free.
 * @param n_floors The number of floors in the tower.
 */
void free_tower(char** tower, int n_floors) {
    if (tower == NULL) {
        return;
    }
    for (int i = 0; i < n_floors; ++i) {
        free(tower[i]);
    }
    free(tower);
}

/**
 * @brief Helper function to print a tower for verification.
 *
 * @param tower The tower to print.
 * @param n_floors The number of floors in the tower.
 */
void print_tower(char** tower, int n_floors) {
    if (tower == NULL || n_floors == 0) {
        printf("[]\n");
        return;
    }
    printf("[\n");
    for (int i = 0; i < n_floors; ++i) {
        printf("  \"%s\"", tower[i]);
        if (i < n_floors - 1) {
            printf(",");
        }
        printf("\n");
    }
    printf("]\n");
}

/**
 * @brief Main function with test cases.
 */
int main() {
    int num_floors;
    char** tower;

    printf("--- Test Case 1: 3 floors ---\n");
    tower = build_tower(3, &num_floors);
    print_tower(tower, num_floors);
    free_tower(tower, num_floors);

    printf("\n--- Test Case 2: 6 floors ---\n");
    tower = build_tower(6, &num_floors);
    print_tower(tower, num_floors);
    free_tower(tower, num_floors);

    printf("\n--- Test Case 3: 1 floor ---\n");
    tower = build_tower(1, &num_floors);
    print_tower(tower, num_floors);
    free_tower(tower, num_floors);

    printf("\n--- Test Case 4: 0 floors (invalid) ---\n");
    tower = build_tower(0, &num_floors);
    print_tower(tower, num_floors);
    free_tower(tower, num_floors);

    printf("\n--- Test Case 5: -5 floors (invalid) ---\n");
    tower = build_tower(-5, &num_floors);
    print_tower(tower, num_floors);
    free_tower(tower, num_floors);

    return 0;
}