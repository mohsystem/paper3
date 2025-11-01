
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

char* generateAndConcatenateFloats() {
    static char buffer[200];
    float float1 = (float)rand() / (float)RAND_MAX;
    float float2 = (float)rand() / (float)RAND_MAX;
    float float3 = (float)rand() / (float)RAND_MAX;
    
    snprintf(buffer, sizeof(buffer), "%f%f%f", float1, float2, float3);
    return buffer;
}

void writeToFile(const char* content, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "%s", content);
        fclose(file);
    } else {
        fprintf(stderr, "Error writing to file: %s\\n", filename);
    }
}

int main() {
    srand(time(NULL));
    
    // Test case 1
    char* result1 = generateAndConcatenateFloats();
    writeToFile(result1, "output1.txt");
    printf("Test 1: %s\\n", result1);
    
    // Test case 2
    char* result2 = generateAndConcatenateFloats();
    writeToFile(result2, "output2.txt");
    printf("Test 2: %s\\n", result2);
    
    // Test case 3
    char* result3 = generateAndConcatenateFloats();
    writeToFile(result3, "output3.txt");
    printf("Test 3: %s\\n", result3);
    
    // Test case 4
    char* result4 = generateAndConcatenateFloats();
    writeToFile(result4, "output4.txt");
    printf("Test 4: %s\\n", result4);
    
    // Test case 5
    char* result5 = generateAndConcatenateFloats();
    writeToFile(result5, "output5.txt");
    printf("Test 5: %s\\n", result5);
    
    return 0;
}
