
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define MAX_INPUT_LENGTH 256
#define MAX_ERROR_LENGTH 512

void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

char* performOperation(const char* operation, double num1, double num2, char* result) {
    char op[50];
    strncpy(op, operation, sizeof(op) - 1);
    op[sizeof(op) - 1] = '\\0';
    toLowerCase(op);
    
    if (strcmp(op, "add") == 0) {
        sprintf(result, "%.6f", num1 + num2);
    } else if (strcmp(op, "subtract") == 0) {
        sprintf(result, "%.6f", num1 - num2);
    } else if (strcmp(op, "multiply") == 0) {
        sprintf(result, "%.6f", num1 * num2);
    } else if (strcmp(op, "divide") == 0) {
        if (num2 == 0) {
            sprintf(result, "Error: Division by zero");
        } else {
            sprintf(result, "%.6f", num1 / num2);
        }
    } else if (strcmp(op, "modulo") == 0) {
        if (num2 == 0) {
            sprintf(result, "Error: Modulo by zero");
        } else {
            sprintf(result, "%.6f", fmod(num1, num2));
        }
    } else if (strcmp(op, "power") == 0) {
        sprintf(result, "%.6f", pow(num1, num2));
    } else {
        sprintf(result, "Error: Invalid operation: %s", operation);
    }
    
    return result;
}

char* parseAndCalculate(const char* input, char* result) {
    if (input == NULL || strlen(input) == 0) {
        sprintf(result, "Error: Input cannot be empty");
        return result;
    }
    
    char operation[50];
    double num1, num2;
    
    int parsed = sscanf(input, "%s %lf %lf", operation, &num1, &num2);
    
    if (parsed != 3) {
        sprintf(result, "Error: Invalid input format. Expected: <operation> <num1> <num2>");
        return result;
    }
    
    return performOperation(operation, num1, num2, result);
}

char* validateAndProcess(char inputs[][MAX_INPUT_LENGTH], int count, char* result) {
    if (inputs == NULL || count == 0) {
        sprintf(result, "Error: Input array is null or empty");
        return result;
    }
    
    char temp[MAX_ERROR_LENGTH];
    result[0] = '\\0';
    
    for (int i = 0; i < count; i++) {
        char singleResult[MAX_ERROR_LENGTH];
        parseAndCalculate(inputs[i], singleResult);
        sprintf(temp, "Input %d: %s\\n", i + 1, singleResult);
        strcat(result, temp);
    }
    
    return result;
}

int main() {
    char result[MAX_ERROR_LENGTH];
    
    printf("=== Test Case 1: Valid Addition ===\\n");
    parseAndCalculate("add 10 5", result);
    printf("%s\\n", result);
    
    printf("\\n=== Test Case 2: Division by Zero ===\\n");
    parseAndCalculate("divide 10 0", result);
    printf("%s\\n", result);
    
    printf("\\n=== Test Case 3: Invalid Operation ===\\n");
    parseAndCalculate("invalid 10 5", result);
    printf("%s\\n", result);
    
    printf("\\n=== Test Case 4: Invalid Number Format ===\\n");
    parseAndCalculate("multiply abc 5", result);
    printf("%s\\n", result);
    
    printf("\\n=== Test Case 5: Batch Processing ===\\n");
    char batchInputs[5][MAX_INPUT_LENGTH] = {
        "add 100 50",
        "subtract 75 25",
        "multiply 8 7",
        "divide 100 4",
        "power 2 8"
    };
    char batchResult[2048];
    validateAndProcess(batchInputs, 5, batchResult);
    printf("%s\\n", batchResult);
    
    return 0;
}
