
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    char string1[50];
    char string2[50];
    char string3[50];
} FloatStrings;

void generateRandomFloatStrings(FloatStrings* result) {
    // Generate three random floats
    float float1 = (float)rand() / RAND_MAX;
    float float2 = (float)rand() / RAND_MAX;
    float float3 = (float)rand() / RAND_MAX;
    
    // Convert to strings using sprintf
    sprintf(result->string1, "%f", float1);
    sprintf(result->string2, "%f", float2);
    sprintf(result->string3, "%f", float3);
}

int main() {
    srand(time(NULL));
    FloatStrings result;
    
    // Test case 1
    generateRandomFloatStrings(&result);
    printf("Test 1: %s, %s, %s\\n", result.string1, result.string2, result.string3);
    
    // Test case 2
    generateRandomFloatStrings(&result);
    printf("Test 2: %s, %s, %s\\n", result.string1, result.string2, result.string3);
    
    // Test case 3
    generateRandomFloatStrings(&result);
    printf("Test 3: %s, %s, %s\\n", result.string1, result.string2, result.string3);
    
    // Test case 4
    generateRandomFloatStrings(&result);
    printf("Test 4: %s, %s, %s\\n", result.string1, result.string2, result.string3);
    
    // Test case 5
    generateRandomFloatStrings(&result);
    printf("Test 5: %s, %s, %s\\n", result.string1, result.string2, result.string3);
    
    return 0;
}
