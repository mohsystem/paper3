
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

// Security: Maximum tower size to prevent resource exhaustion
#define MAX_FLOORS 10000

// Security: Structure to manage tower with explicit size tracking
typedef struct {
    char** floors;
    int count;
} Tower;

// Security: Safe string creation with bounds checking
char* createFloorString(int width) {
    // Security: Validate width to prevent negative or excessive allocation
    if (width <= 0 || width > INT_MAX / 2) {
        return NULL;
    }
    
    // Security: Allocate with explicit size check, +1 for null terminator
    char* str = (char*)calloc(width + 1, sizeof(char));
    if (str == NULL) {
        return NULL; // Memory allocation failed
    }
    
    // Security: Ensure null termination
    str[width] = '\\0';
    return str;
}

// Security: Free tower memory safely
void freeTower(Tower* tower) {
    if (tower == NULL) {
        return;
    }
    
    if (tower->floors != NULL) {
        for (int i = 0; i < tower->count; i++) {
            if (tower->floors[i] != NULL) {
                // Security: Clear memory before freeing (not critical for this data)
                free(tower->floors[i]);
                tower->floors[i] = NULL;
            }
        }
        free(tower->floors);
        tower->floors = NULL;
    }
    tower->count = 0;
}

Tower buildTower(int nFloors) {
    Tower tower = {NULL, 0};
    
    // Security: Validate input range
    if (nFloors <= 0) {
        fprintf(stderr, "Error: Number of floors must be positive\\n");
        return tower;
    }
    
    if (nFloors > MAX_FLOORS) {
        fprintf(stderr, "Error: Number of floors exceeds maximum allowed\\n");
        return tower;
    }
    
    // Security: Check for integer overflow in width calculation
    if (nFloors > INT_MAX / 2) {
        fprintf(stderr, "Error: Floor calculation would overflow\\n");
        return tower;
    }
    
    int width = 2 * nFloors - 1;
    
    // Security: Allocate array with size check
    tower.floors = (char**)calloc(nFloors, sizeof(char*));
    if (tower.floors == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return tower;
    }
    
    // Build each floor
    for (int i = 0; i < nFloors; i++) {
        // Security: Check for overflow in stars calculation
        if (i > INT_MAX / 2) {
            fprintf(stderr, "Error: Stars calculation would overflow\\n");
            freeTower(&tower);
            return tower;
        }
        
        int stars = 2 * i + 1;
        int spaces = (width - stars) / 2;
        
        // Security: Validate dimensions are non-negative
        if (spaces < 0 || stars < 0) {
            fprintf(stderr, "Error: Invalid floor dimensions\\n");
            freeTower(&tower);
            return tower;
        }
        
        // Security: Create floor string with bounds checking
        char* floor = createFloorString(width);
        if (floor == NULL) {
            fprintf(stderr, "Error: Failed to create floor string\\n");
            freeTower(&tower);
            return tower;
        }
        
        // Security: Manually fill with bounds checking
        int pos = 0;
        for (int j = 0; j < spaces && pos < width; j++, pos++) {
            floor[pos] = ' ';
        }
        for (int j = 0; j < stars && pos < width; j++, pos++) {
            floor[pos] = '*';
        }
        for (int j = 0; j < spaces && pos < width; j++, pos++) {
            floor[pos] = ' ';
        }
        
        tower.floors[i] = floor;
        tower.count++;
    }
    
    return tower;
}

int main(void) {
    // Test case 1: Basic 3-floor tower
    printf("Test 1 - 3 floors:\\n");
    Tower tower1 = buildTower(3);
    if (tower1.floors != NULL) {
        for (int i = 0; i < tower1.count; i++) {
            printf("\\"%s\\"\\n", tower1.floors[i]);
        }
        freeTower(&tower1);
    }
    printf("\\n");
    
    // Test case 2: 6-floor tower
    printf("Test 2 - 6 floors:\\n");
    Tower tower2 = buildTower(6);
    if (tower2.floors != NULL) {
        for (int i = 0; i < tower2.count; i++) {
            printf("\\"%s\\"\\n", tower2.floors[i]);
        }
        freeTower(&tower2);
    }
    printf("\\n");
    
    // Test case 3: Single floor
    printf("Test 3 - 1 floor:\\n");
    Tower tower3 = buildTower(1);
    if (tower3.floors != NULL) {
        for (int i = 0; i < tower3.count; i++) {
            printf("\\"%s\\"\\n", tower3.floors[i]);
        }
        freeTower(&tower3);
    }
    printf("\\n");
    
    // Test case 4: Invalid input (zero)
    printf("Test 4 - 0 floors (should fail):\\n");
    Tower tower4 = buildTower(0);
    if (tower4.floors != NULL) {
        for (int i = 0; i < tower4.count; i++) {
            printf("\\"%s\\"\\n", tower4.floors[i]);
        }
        freeTower(&tower4);
    }
    printf("\\n");
    
    // Test case 5: Invalid input (negative)
    printf("Test 5 - negative floors (should fail):\\n");
    Tower tower5 = buildTower(-5);
    if (tower5.floors != NULL) {
        for (int i = 0; i < tower5.count; i++) {
            printf("\\"%s\\"\\n", tower5.floors[i]);
        }
        freeTower(&tower5);
    }
    printf("\\n");
    
    return 0;
}
