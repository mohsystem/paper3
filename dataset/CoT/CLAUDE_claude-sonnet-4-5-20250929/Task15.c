
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FLOORS 1000

typedef struct {
    char** floors;
    int count;
} Tower;

Tower* buildTower(int nFloors) {
    // Input validation
    if (nFloors <= 0) {
        Tower* tower = (Tower*)malloc(sizeof(Tower));
        if (tower == NULL) {
            return NULL;
        }
        tower->floors = NULL;
        tower->count = 0;
        return tower;
    }
    
    if (nFloors > MAX_FLOORS) {
        fprintf(stderr, "Error: Number of floors exceeds maximum allowed\\n");
        return NULL;
    }
    
    Tower* tower = (Tower*)malloc(sizeof(Tower));
    if (tower == NULL) {
        return NULL;
    }
    
    tower->count = nFloors;
    tower->floors = (char**)malloc(nFloors * sizeof(char*));
    if (tower->floors == NULL) {
        free(tower);
        return NULL;
    }
    
    int width = 2 * nFloors - 1;
    
    for (int i = 0; i < nFloors; i++) {
        int stars = 2 * i + 1;
        int spaces = (width - stars) / 2;
        
        // Allocate memory for floor string (+1 for null terminator)
        tower->floors[i] = (char*)malloc((width + 1) * sizeof(char));
        if (tower->floors[i] == NULL) {
            // Free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(tower->floors[j]);
            }
            free(tower->floors);
            free(tower);
            return NULL;
        }
        
        int pos = 0;
        // Add leading spaces
        for (int j = 0; j < spaces; j++) {
            tower->floors[i][pos++] = ' ';
        }
        // Add stars
        for (int j = 0; j < stars; j++) {
            tower->floors[i][pos++] = '*';
        }
        // Add trailing spaces
        for (int j = 0; j < spaces; j++) {
            tower->floors[i][pos++] = ' ';
        }
        tower->floors[i][pos] = '\\0'; // Null terminate
    }
    
    return tower;
}

void freeTower(Tower* tower) {
    if (tower != NULL) {
        if (tower->floors != NULL) {
            for (int i = 0; i < tower->count; i++) {
                free(tower->floors[i]);
            }
            free(tower->floors);
        }
        free(tower);
    }
}

void printTower(Tower* tower) {
    if (tower != NULL && tower->floors != NULL) {
        for (int i = 0; i < tower->count; i++) {
            printf("\\"%s\\"\\n", tower->floors[i]);
        }
    }
}

int main() {
    // Test case 1: 3 floors
    printf("Test 1 - 3 floors:\\n");
    Tower* result1 = buildTower(3);
    printTower(result1);
    freeTower(result1);
    
    // Test case 2: 6 floors
    printf("\\nTest 2 - 6 floors:\\n");
    Tower* result2 = buildTower(6);
    printTower(result2);
    freeTower(result2);
    
    // Test case 3: 1 floor
    printf("\\nTest 3 - 1 floor:\\n");
    Tower* result3 = buildTower(1);
    printTower(result3);
    freeTower(result3);
    
    // Test case 4: 0 floors (edge case)
    printf("\\nTest 4 - 0 floors:\\n");
    Tower* result4 = buildTower(0);
    printf("Empty array, length: %d\\n", result4 ? result4->count : 0);
    freeTower(result4);
    
    // Test case 5: 10 floors
    printf("\\nTest 5 - 10 floors:\\n");
    Tower* result5 = buildTower(10);
    printTower(result5);
    freeTower(result5);
    
    return 0;
}
