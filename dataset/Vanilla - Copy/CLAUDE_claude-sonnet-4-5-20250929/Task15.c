
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** buildTower(int nFloors, int* size) {
    *size = nFloors;
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
    printf("Test case 1: 3 floors\\n");
    char** tower1 = buildTower(3, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower1[i]);
    }
    freeTower(tower1, size);
    printf("\\n");
    
    // Test case 2
    printf("Test case 2: 6 floors\\n");
    char** tower2 = buildTower(6, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower2[i]);
    }
    freeTower(tower2, size);
    printf("\\n");
    
    // Test case 3
    printf("Test case 3: 1 floor\\n");
    char** tower3 = buildTower(1, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower3[i]);
    }
    freeTower(tower3, size);
    printf("\\n");
    
    // Test case 4
    printf("Test case 4: 5 floors\\n");
    char** tower4 = buildTower(5, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower4[i]);
    }
    freeTower(tower4, size);
    printf("\\n");
    
    // Test case 5
    printf("Test case 5: 10 floors\\n");
    char** tower5 = buildTower(10, &size);
    for (int i = 0; i < size; i++) {
        printf("\\"%s\\"\\n", tower5[i]);
    }
    freeTower(tower5, size);
    
    return 0;
}
