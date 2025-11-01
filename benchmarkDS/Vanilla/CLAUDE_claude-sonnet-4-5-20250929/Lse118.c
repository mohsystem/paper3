
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void writeRandomFloatsToFile(const char* filename) {
    // Generate 3 random floats
    float float1 = (float)rand() / (float)RAND_MAX;
    float float2 = (float)rand() / (float)RAND_MAX;
    float float3 = (float)rand() / (float)RAND_MAX;
    
    // Convert them to strings and concatenate
    char str1[50], str2[50], str3[50];
    sprintf(str1, "%f", float1);
    sprintf(str2, "%f", float2);
    sprintf(str3, "%f", float3);
    
    // Concatenate into a single string
    char concatenated[200];
    sprintf(concatenated, "%s%s%s", str1, str2, str3);
    
    // Write the string to a file
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "%s", concatenated);
        fclose(file);
    } else {
        fprintf(stderr, "Unable to open file: %s\\n", filename);
    }
}

int main() {
    // Seed the random number generator
    srand(time(NULL));
    
    // Test case 1
    writeRandomFloatsToFile("output1.txt");
    
    // Test case 2
    writeRandomFloatsToFile("output2.txt");
    
    // Test case 3
    writeRandomFloatsToFile("output3.txt");
    
    // Test case 4
    writeRandomFloatsToFile("output4.txt");
    
    // Test case 5
    writeRandomFloatsToFile("output5.txt");
    
    printf("Files created successfully!\\n");
    
    return 0;
}
