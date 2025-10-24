#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * Attempts to parse two strings as doubles and divide the first by the second.
 * C does not have exceptions, so error handling is done via return codes.
 *
 * @param a The numerator as a string.
 * @param b The denominator as a string.
 * @param result A pointer to a double where the result will be stored on success.
 * @return 0 on success.
 *         1 if there is an invalid number format.
 *         2 if there is division by zero.
 *         3 if a number is out of range (overflow/underflow).
 */
int divideStrings(const char* a, const char* b, double* result) {
    char* end_a;
    char* end_b;
    double numerator, denominator;

    // Reset errno before the call
    errno = 0;

    // Convert strings to doubles
    numerator = strtod(a, &end_a);
    denominator = strtod(b, &end_b);

    // Check for conversion errors
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Number is out of range.\n");
        return 3;
    }

    // Check if the entire string was consumed. If not, it's an invalid format.
    // We also check that the string was not empty to begin with.
    if (end_a == a || *end_a != '\0' || end_b == b || *end_b != '\0') {
        fprintf(stderr, "Error: Invalid number format.\n");
        return 1;
    }

    // Check for division by zero
    if (denominator == 0.0) {
        fprintf(stderr, "Error: Division by zero is not allowed.\n");
        return 2;
    }

    *result = numerator / denominator;
    return 0; // Success
}

int main() {
    printf("--- C Test Cases ---\n");
    double result;
    int status;

    // Test Case 1: Valid division
    printf("\nTest Case 1: divideStrings(\"10\", \"2\")\n");
    status = divideStrings("10", "2", &result);
    if (status == 0) {
        printf("Result: %f\n", result);
    }

    // Test Case 2: Division by zero
    printf("\nTest Case 2: divideStrings(\"5\", \"0\")\n");
    status = divideStrings("5", "0", &result);
    if (status == 0) {
        printf("Result: %f\n", result);
    }

    // Test Case 3: Invalid numerator
    printf("\nTest Case 3: divideStrings(\"abc\", \"5\")\n");
    status = divideStrings("abc", "5", &result);
    if (status == 0) {
        printf("Result: %f\n", result);
    }

    // Test Case 4: Invalid denominator
    printf("\nTest Case 4: divideStrings(\"10\", \"xyz\")\n");
    status = divideStrings("10", "xyz", &result);
    if (status == 0) {
        printf("Result: %f\n", result);
    }
    
    // Test Case 5: Valid floating-point division
    printf("\nTest Case 5: divideStrings(\"7.5\", \"2.5\")\n");
    status = divideStrings("7.5", "2.5", &result);
    if (status == 0) {
        printf("Result: %f\n", result);
    }

    return 0;
}