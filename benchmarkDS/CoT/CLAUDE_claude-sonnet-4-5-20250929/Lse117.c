
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_random_float_strings(char* str_a, char* str_b, char* str_c) {
    // Generate three random floats
    float float_a = (float)rand() / (float)RAND_MAX;
    float float_b = (float)rand() / (float)RAND_MAX;
    float float_c = (float)rand() / (float)RAND_MAX;
    
    // Convert floats to strings with secure buffer sizes
    snprintf(str_a, 50, "%f", float_a);
    snprintf(str_b, 50, "%f", float_b);
    snprintf(str_c, 50, "%f", float_c);
}

int main() {
    // Seed the random number generator
    srand(time(NULL));
    
    // Allocate buffers with sufficient size
    char str_a[50], str_b[50], str_c[50];
    
    // Test case 1
    printf("Test Case 1:\\n");
    generate_random_float_strings(str_a, str_b, str_c);
    printf("str_a: %s\\n", str_a);
    printf("str_b: %s\\n", str_b);
    printf("str_c: %s\\n", str_c);
    printf("\\n");
    
    // Test case 2
    printf("Test Case 2:\\n");
    generate_random_float_strings(str_a, str_b, str_c);
    printf("str_a: %s\\n", str_a);
    printf("str_b: %s\\n", str_b);
    printf("str_c: %s\\n", str_c);
    printf("\\n");
    
    // Test case 3
    printf("Test Case 3:\\n");
    generate_random_float_strings(str_a, str_b, str_c);
    printf("str_a: %s\\n", str_a);
    printf("str_b: %s\\n", str_b);
    printf("str_c: %s\\n", str_c);
    printf("\\n");
    
    // Test case 4
    printf("Test Case 4:\\n");
    generate_random_float_strings(str_a, str_b, str_c);
    printf("str_a: %s\\n", str_a);
    printf("str_b: %s\\n", str_b);
    printf("str_c: %s\\n", str_c);
    printf("\\n");
    
    // Test case 5
    printf("Test Case 5:\\n");
    generate_random_float_strings(str_a, str_b, str_c);
    printf("str_a: %s\\n", str_a);
    printf("str_b: %s\\n", str_b);
    printf("str_c: %s\\n", str_c);
    
    return 0;
}
