
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

/* Function to perform arithmetic operation with input validation
 * Returns 0 on success, -1 on error
 * result pointer must be valid and will contain the result on success
 */
int calculate(double num1, double num2, char op, double* result) {
    /* Input validation: ensure result pointer is valid */
    if (result == NULL) {
        fprintf(stderr, "Error: NULL result pointer\\n");
        return -1;
    }
    
    /* Initialize result to safe value */
    *result = 0.0;
    
    /* Validate operator input - only allow specific operators */
    if (op != '+' && op != '-' && op != '*' && op != '/') {
        fprintf(stderr, "Error: Invalid operator. Only +, -, *, / are allowed.\\n");
        return -1;
    }
    
    /* Check for NaN or Infinity to prevent undefined behavior */
    if (isnan(num1) || isnan(num2) || isinf(num1) || isinf(num2)) {
        fprintf(stderr, "Error: Invalid number - NaN or Infinity not allowed.\\n");
        return -1;
    }
    
    switch(op) {
        case '+':
            /* Check for overflow in addition */
            if ((num2 > 0 && num1 > DBL_MAX - num2) ||
                (num2 < 0 && num1 < -DBL_MAX - num2)) {
                fprintf(stderr, "Error: Addition overflow detected.\\n");
                return -1;
            }
            *result = num1 + num2;
            break;
            
        case '-':
            /* Check for overflow in subtraction */
            if ((num2 < 0 && num1 > DBL_MAX + num2) ||
                (num2 > 0 && num1 < -DBL_MAX + num2)) {
                fprintf(stderr, "Error: Subtraction overflow detected.\\n");
                return -1;
            }
            *result = num1 - num2;
            break;
            
        case '*':
            /* Check for overflow in multiplication */
            if (num1 != 0.0 && num2 != 0.0) {
                if (fabs(num1) > DBL_MAX / fabs(num2)) {
                    fprintf(stderr, "Error: Multiplication overflow detected.\\n");
                    return -1;
                }
            }
            *result = num1 * num2;
            break;
            
        case '/':
            /* Critical: Prevent division by zero */
            if (num2 == 0.0) {
                fprintf(stderr, "Error: Division by zero is not allowed.\\n");
                return -1;
            }
            *result = num1 / num2;
            break;
            
        default:
            /* Should never reach here due to validation above */
            fprintf(stderr, "Error: Unknown operator.\\n");
            return -1;
    }
    
    /* Validate result is not NaN or Infinity after operation */
    if (isnan(*result) || isinf(*result)) {
        fprintf(stderr, "Error: Operation resulted in invalid value.\\n");
        return -1;
    }
    
    return 0;
}

int main(void) {
    double result = 0.0;
    int status = 0;
    
    /* Test Case 1: Addition */
    printf("Test Case 1: 10 + 5\\n");
    status = calculate(10.0, 5.0, '+', &result);
    if (status == 0) {
        printf("Result: %.2f\\n", result);
    }
    printf("\\n");
    
    /* Test Case 2: Subtraction */
    printf("Test Case 2: 20 - 8\\n");
    status = calculate(20.0, 8.0, '-', &result);
    if (status == 0) {
        printf("Result: %.2f\\n", result);
    }
    printf("\\n");
    
    /* Test Case 3: Multiplication */
    printf("Test Case 3: 7 * 6\\n");
    status = calculate(7.0, 6.0, '*', &result);
    if (status == 0) {
        printf("Result: %.2f\\n", result);
    }
    printf("\\n");
    
    /* Test Case 4: Division */
    printf("Test Case 4: 15 / 3\\n");
    status = calculate(15.0, 3.0, '/', &result);
    if (status == 0) {
        printf("Result: %.2f\\n", result);
    }
    printf("\\n");
    
    /* Test Case 5: Division by zero (expected to fail gracefully) */
    printf("Test Case 5: 10 / 0 (Division by zero)\\n");
    status = calculate(10.0, 0.0, '/', &result);
    if (status == 0) {
        printf("Result: %.2f\\n", result);
    }
    printf("\\n");
    
    return 0;
}
