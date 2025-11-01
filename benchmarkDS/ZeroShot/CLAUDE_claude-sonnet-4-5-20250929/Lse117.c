
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void floatToString(float value, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%f", value);
}

float randomFloat() {
    return (float)rand() / (float)RAND_MAX;
}

int main() {
    srand(time(NULL));
    char str_a[50], str_b[50], str_c[50];
    
    // Test case 1
    float a1 = randomFloat();
    float b1 = randomFloat();
    float c1 = randomFloat();
    floatToString(a1, str_a, sizeof(str_a));
    floatToString(b1, str_b, sizeof(str_b));
    floatToString(c1, str_c, sizeof(str_c));
    printf("Test 1: %s, %s, %s\\n", str_a, str_b, str_c);
    
    // Test case 2
    float a2 = randomFloat();
    float b2 = randomFloat();
    float c2 = randomFloat();
    floatToString(a2, str_a, sizeof(str_a));
    floatToString(b2, str_b, sizeof(str_b));
    floatToString(c2, str_c, sizeof(str_c));
    printf("Test 2: %s, %s, %s\\n", str_a, str_b, str_c);
    
    // Test case 3
    float a3 = randomFloat();
    float b3 = randomFloat();
    float c3 = randomFloat();
    floatToString(a3, str_a, sizeof(str_a));
    floatToString(b3, str_b, sizeof(str_b));
    floatToString(c3, str_c, sizeof(str_c));
    printf("Test 3: %s, %s, %s\\n", str_a, str_b, str_c);
    
    // Test case 4
    float a4 = randomFloat();
    float b4 = randomFloat();
    float c4 = randomFloat();
    floatToString(a4, str_a, sizeof(str_a));
    floatToString(b4, str_b, sizeof(str_b));
    floatToString(c4, str_c, sizeof(str_c));
    printf("Test 4: %s, %s, %s\\n", str_a, str_b, str_c);
    
    // Test case 5
    float a5 = randomFloat();
    float b5 = randomFloat();
    float c5 = randomFloat();
    floatToString(a5, str_a, sizeof(str_a));
    floatToString(b5, str_b, sizeof(str_b));
    floatToString(c5, str_c, sizeof(str_c));
    printf("Test 5: %s, %s, %s\\n", str_a, str_b, str_c);
    
    return 0;
}
