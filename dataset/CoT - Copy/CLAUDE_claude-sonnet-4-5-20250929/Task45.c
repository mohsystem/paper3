
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#define MAX_INPUT_LENGTH 1000
#define SUCCESS 0
#define ERROR -1

typedef struct {
    int code;
    char message[256];
} ErrorInfo;

// Secure string trimming
char* trim(char* str) {
    if (str == NULL) return NULL;
    
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\\0';
    
    return str;
}

// Secure string to lowercase
void toLower(char* str) {
    if (str == NULL) return;
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

// Secure calculator with input validation
int calculator(const char* operation, double num1, double num2, double* result, ErrorInfo* error) {
    if (operation == NULL || strlen(operation) == 0) {
        if (error) {
            error->code = ERROR;
            strcpy(error->message, "Operation cannot be null or empty");
        }
        return ERROR;
    }
    
    if (isnan(num1) || isnan(num2) || isinf(num1) || isinf(num2)) {
        if (error) {
            error->code = ERROR;
            strcpy(error->message, "Invalid number input");
        }
        return ERROR;
    }
    
    char op[256];
    strncpy(op, operation, sizeof(op) - 1);
    op[sizeof(op) - 1] = '\\0';
    toLower(trim(op));
    
    if (strcmp(op, "add") == 0) {
        *result = num1 + num2;
    } else if (strcmp(op, "subtract") == 0) {
        *result = num1 - num2;
    } else if (strcmp(op, "multiply") == 0) {
        *result = num1 * num2;
    } else if (strcmp(op, "divide") == 0) {
        if (num2 == 0) {
            if (error) {
                error->code = ERROR;
                strcpy(error->message, "Division by zero is not allowed");
            }
            return ERROR;
        }
        *result = num1 / num2;
    } else {
        if (error) {
            error->code = ERROR;
            snprintf(error->message, sizeof(error->message), "Invalid operation: %s", operation);
        }
        return ERROR;
    }
    
    if (error) error->code = SUCCESS;
    return SUCCESS;
}

// Secure input validator
int validateInput(const char* input, int* isValid, ErrorInfo* error) {
    if (input == NULL) {
        if (error) {
            error->code = ERROR;
            strcpy(error->message, "Input cannot be null");
        }
        return ERROR;
    }
    
    if (strlen(input) > MAX_INPUT_LENGTH) {
        if (error) {
            error->code = ERROR;
            strcpy(error->message, "Input exceeds maximum allowed length");
        }
        return ERROR;
    }
    
    *isValid = 1;
    for (size_t i = 0; i < strlen(input); i++) {
        if (!isalnum((unsigned char)input[i]) && !isspace((unsigned char)input[i])) {
            *isValid = 0;
            break;
        }
    }
    
    if (error) error->code = SUCCESS;
    return SUCCESS;
}

// Secure array processor
int processArray(const int* arr, int arrSize, int index, int* result, ErrorInfo* error) {
    if (arr == NULL) {
        if (error) {
            error->code = ERROR;
            strcpy(error->message, "Array cannot be null");
        }
        return ERROR;
    }
    
    if (arrSize == 0) {
        if (error) {
            error->code = ERROR;
            strcpy(error->message, "Array cannot be empty");
        }
        return ERROR;
    }
    
    if (index < 0 || index >= arrSize) {
        if (error) {
            error->code = ERROR;
            snprintf(error->message, sizeof(error->message), 
                "Index %d is out of bounds for array length %d", index, arrSize);
        }
        return ERROR;
    }
    
    *result = arr[index] * 2;
    if (error) error->code = SUCCESS;
    return SUCCESS;
}

// Secure string parser
int parseSecureInteger(const char* input, int* result, ErrorInfo* error) {
    if (input == NULL || strlen(input) == 0) {
        if (error) {
            error->code = ERROR;
            strcpy(error->message, "Input cannot be null or empty");
        }
        return ERROR;
    }
    
    char buffer[256];
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\\0';
    char* trimmed = trim(buffer);
    
    if (strlen(trimmed) > 10) {
        if (error) {
            error->code = ERROR;
            strcpy(error->message, "Input string too long for integer parsing");
        }
        return ERROR;
    }
    
    char* endptr;
    errno = 0;
    long val = strtol(trimmed, &endptr, 10);
    
    if (errno != 0 || endptr == trimmed || *endptr != '\\0') {
        if (error) {
            error->code = ERROR;
            snprintf(error->message, sizeof(error->message), "Invalid integer format: %s", trimmed);
        }
        return ERROR;
    }
    
    *result = (int)val;
    if (error) error->code = SUCCESS;
    return SUCCESS;
}

int main() {
    ErrorInfo error;
    
    printf("=== Test Case 1: Calculator Operations ===\\n");
    double calcResult;
    if (calculator("add", 10, 5, &calcResult, &error) == SUCCESS) {
        printf("Add 10 + 5 = %.2f\\n", calcResult);
    }
    if (calculator("divide", 20, 4, &calcResult, &error) == SUCCESS) {
        printf("Divide 20 / 4 = %.2f\\n", calcResult);
    }
    
    printf("\\n=== Test Case 2: Division by Zero ===\\n");
    if (calculator("divide", 10, 0, &calcResult, &error) == ERROR) {
        fprintf(stderr, "Caught expected error: %s\\n", error.message);
    }
    
    printf("\\n=== Test Case 3: Input Validation ===\\n");
    int isValid;
    if (validateInput("Hello123", &isValid, &error) == SUCCESS) {
        printf("Valid input 'Hello123': %d\\n", isValid);
    }
    if (validateInput("Hello@123", &isValid, &error) == SUCCESS) {
        printf("Invalid input 'Hello@123': %d\\n", isValid);
    }
    
    printf("\\n=== Test Case 4: Array Processing ===\\n");
    int testArray[] = {1, 2, 3, 4, 5};
    int arrayResult;
    if (processArray(testArray, 5, 2, &arrayResult, &error) == SUCCESS) {
        printf("Process array[2] = %d\\n", arrayResult);
    }
    if (processArray(testArray, 5, 10, &arrayResult, &error) == ERROR) {
        fprintf(stderr, "Caught expected error: %s\\n", error.message);
    }
    
    printf("\\n=== Test Case 5: String Parsing ===\\n");
    int parseResult;
    if (parseSecureInteger("123", &parseResult, &error) == SUCCESS) {
        printf("Parse '123' = %d\\n", parseResult);
    }
    if (parseSecureInteger("abc", &parseResult, &error) == ERROR) {
        fprintf(stderr, "Caught expected error: %s\\n", error.message);
    }
    
    return 0;
}
