
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

char* generateAndConcatenateFloats() {
    static char result[300];
    char str1[100], str2[100], str3[100];
    
    float float1 = ((float)rand() / RAND_MAX) * 100.0f;
    float float2 = ((float)rand() / RAND_MAX) * 100.0f;
    float float3 = ((float)rand() / RAND_MAX) * 100.0f;
    
    snprintf(str1, sizeof(str1), "%f", float1);
    snprintf(str2, sizeof(str2), "%f", float2);
    snprintf(str3, sizeof(str3), "%f", float3);
    
    snprintf(result, sizeof(result), "%s%s%s", str1, str2, str3);
    
    return result;
}

void writeToFile(const char* content, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "%s", content);
        fclose(file);
    } else {
        fprintf(stderr, "Error opening file: %s\\n", filename);
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
