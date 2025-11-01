
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generateRandomFloatStrings() {
    // Buffer size: sign(1) + digits(38) + decimal(1) + precision(6) + null(1) = 50 is safe
    const int BUFFER_SIZE = 50;
    char string1[BUFFER_SIZE];
    char string2[BUFFER_SIZE];
    char string3[BUFFER_SIZE];
    
    // Generate three random floats (0.0 to 1.0)
    float float1 = (float)rand() / (float)RAND_MAX;
    float float2 = (float)rand() / (float)RAND_MAX;
    float float3 = (float)rand() / (float)RAND_MAX;
    
    // Convert to strings using snprintf (secure version of sprintf)
    snprintf(string1, BUFFER_SIZE, "%f", float1);
    snprintf(string2, BUFFER_SIZE, "%f", float2);
    snprintf(string3, BUFFER_SIZE, "%f", float3);
    
    printf("Float 1: %s\\n", string1);
    printf("Float 2: %s\\n", string2);
    printf("Float 3: %s\\n", string3);
}

int main() {
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    
    printf("Test Case 1:\\n");
    generateRandomFloatStrings();
    
    printf("\\nTest Case 2:\\n");
    generateRandomFloatStrings();
    
    printf("\\nTest Case 3:\\n");
    generateRandomFloatStrings();
    
    printf("\\nTest Case 4:\\n");
    generateRandomFloatStrings();
    
    printf("\\nTest Case 5:\\n");
    generateRandomFloatStrings();
    
    return 0;
}
