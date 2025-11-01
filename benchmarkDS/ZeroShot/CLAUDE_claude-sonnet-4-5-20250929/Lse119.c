
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generateRandomFloatStrings(char* string1, char* string2, char* string3) {
    float float1 = (float)rand() / RAND_MAX;
    float float2 = (float)rand() / RAND_MAX;
    float float3 = (float)rand() / RAND_MAX;
    
    sprintf(string1, "%f", float1);
    sprintf(string2, "%f", float2);
    sprintf(string3, "%f", float3);
}

int main() {
    srand((unsigned int)time(NULL));
    
    char string1[50], string2[50], string3[50];
    
    // Test case 1
    printf("Test case 1:\\n");
    generateRandomFloatStrings(string1, string2, string3);
    printf("String 1: %s\\n", string1);
    printf("String 2: %s\\n", string2);
    printf("String 3: %s\\n", string3);
    printf("\\n");
    
    // Test case 2
    printf("Test case 2:\\n");
    generateRandomFloatStrings(string1, string2, string3);
    printf("String 1: %s\\n", string1);
    printf("String 2: %s\\n", string2);
    printf("String 3: %s\\n", string3);
    printf("\\n");
    
    // Test case 3
    printf("Test case 3:\\n");
    generateRandomFloatStrings(string1, string2, string3);
    printf("String 1: %s\\n", string1);
    printf("String 2: %s\\n", string2);
    printf("String 3: %s\\n", string3);
    printf("\\n");
    
    // Test case 4
    printf("Test case 4:\\n");
    generateRandomFloatStrings(string1, string2, string3);
    printf("String 1: %s\\n", string1);
    printf("String 2: %s\\n", string2);
    printf("String 3: %s\\n", string3);
    printf("\\n");
    
    // Test case 5
    printf("Test case 5:\\n");
    generateRandomFloatStrings(string1, string2, string3);
    printf("String 1: %s\\n", string1);
    printf("String 2: %s\\n", string2);
    printf("String 3: %s\\n", string3);
    
    return 0;
}
