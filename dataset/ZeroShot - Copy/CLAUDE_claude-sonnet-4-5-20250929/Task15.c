
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** towerBuilder(int nFloors, int* returnSize) {
    if (nFloors <= 0) {
        *returnSize = 0;
        return NULL;
    }
    
    *returnSize = nFloors;
    char** tower = (char**)malloc(nFloors * sizeof(char*));
    int maxWidth = 2 * nFloors - 1;
    
    for (int i = 0; i < nFloors; i++) {
        int stars = 2 * i + 1;
        int spaces = (maxWidth - stars) / 2;
        
        tower[i] = (char*)malloc((maxWidth + 1) * sizeof(char));
        int pos = 0;
        
        for (int j = 0; j < spaces; j++) {
            tower[i][pos++] = ' ';
        }
        for (int j = 0; j < stars; j++) {
            tower[i][pos++] = '*';
        }
        for (int j = 0; j < spaces; j++) {
            tower[i][pos++] = ' ';
        }
        tower[i][pos] = '\\0';
    }
    
    return tower;
}

void freeTower(char** tower, int size) {
    for (int i = 0; i < size; i++) {
        free(tower[i]);
    }
    free(tower);
}

int main() {
    int size;
    
    // Test case 1
    printf("Test case 1 (3 floors):\\n");
    char** tower1 = towerBuilder(3, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower1[i]);
    }
    printf("\\n");
    freeTower(tower1, size);
    
    // Test case 2
    printf("Test case 2 (6 floors):\\n");
    char** tower2 = towerBuilder(6, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower2[i]);
    }
    printf("\\n");
    freeTower(tower2, size);
    
    // Test case 3
    printf("Test case 3 (1 floor):\\n");
    char** tower3 = towerBuilder(1, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower3[i]);
    }
    printf("\\n");
    freeTower(tower3, size);
    
    // Test case 4
    printf("Test case 4 (5 floors):\\n");
    char** tower4 = towerBuilder(5, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower4[i]);
    }
    printf("\\n");
    freeTower(tower4, size);
    
    // Test case 5
    printf("Test case 5 (10 floors):\\n");
    char** tower5 = towerBuilder(10, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower5[i]);
    }
    freeTower(tower5, size);
    
    return 0;
}
