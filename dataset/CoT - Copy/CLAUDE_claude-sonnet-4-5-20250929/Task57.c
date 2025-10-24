
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_DIGITS 35660  // Maximum digits for factorial of 10000

// Function to multiply large number with integer
void multiply(int* result, int* size, int x) {
    int carry = 0;
    
    for (int i = 0; i < *size; i++) {
        int prod = result[i] * x + carry;
        result[i] = prod % 10;
        carry = prod / 10;
    }
    
    while (carry > 0) {
        result[*size] = carry % 10;
        carry = carry / 10;
        (*size)++;
    }
}

// Function to calculate factorial with security validations
char* calculateFactorial(int n) {
    // Input validation to prevent negative numbers
    if (n < 0) {
        fprintf(stderr, "Error: Factorial is not defined for negative numbers\\n");
        return NULL;
    }
    
    // Check for reasonable upper limit to prevent resource exhaustion
    if (n > 10000) {
        fprintf(stderr, "Error: Input too large - maximum value is 10000\\n");
        return NULL;
    }
    
    // Allocate memory for result array
    int* result = (int*)calloc(MAX_DIGITS, sizeof(int));
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    result[0] = 1;
    int size = 1;
    
    // Calculate factorial
    for (int x = 2; x <= n; x++) {
        multiply(result, &size, x);
    }
    
    // Convert result to string (with bounds checking)
    char* factorialStr = (char*)malloc((size + 1) * sizeof(char));
    if (factorialStr == NULL) {
        free(result);
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    for (int i = 0; i < size; i++) {
        factorialStr[i] = '0' + result[size - 1 - i];
    }
    factorialStr[size] = '\\0';
    
    free(result);
    return factorialStr;
}

int main() {
    // Test cases
    char* result;
    
    printf("Test Case 1: Factorial of 0 = ");
    result = calculateFactorial(0);
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("Test Case 2: Factorial of 1 = ");
    result = calculateFactorial(1);
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("Test Case 3: Factorial of 5 = ");
    result = calculateFactorial(5);
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("Test Case 4: Factorial of 10 = ");
    result = calculateFactorial(10);
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("Test Case 5: Factorial of 20 = ");
    result = calculateFactorial(20);
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    // Test error handling
    printf("Test Case 6 (Error): Factorial of -5\\n");
    result = calculateFactorial(-5);
    if (result) {
        free(result);
    }
    
    return 0;
}
