
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <regex.h>

#define MAX_INPUT_LENGTH 1000
#define MAX_OUTPUT_LENGTH 2000

char* trim(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\\0';
    return str;
}

void replaceChar(char* str, char find, const char* replace, char* output) {
    int i = 0, j = 0;
    int replaceLen = strlen(replace);
    
    while (str[i] != '\\0') {
        if (str[i] == find) {
            strcpy(&output[j], replace);
            j += replaceLen;
        } else {
            output[j++] = str[i];
        }
        i++;
    }
    output[j] = '\\0';
}

char* validateAndSanitizeText(const char* input, char* output) {
    if (input == NULL || strlen(input) == 0) {
        strcpy(output, "Error: Input is null or empty");
        return output;
    }
    
    if (strlen(input) > MAX_INPUT_LENGTH) {
        strcpy(output, "Error: Input exceeds maximum length");
        return output;
    }
    
    char trimmed[MAX_INPUT_LENGTH + 1];
    strcpy(trimmed, input);
    char* trimmedPtr = trim(trimmed);
    
    regex_t regex;
    int reti = regcomp(&regex, "^[a-zA-Z0-9 .,!?-]+$", REG_EXTENDED);
    reti = regexec(&regex, trimmedPtr, 0, NULL, 0);
    regfree(&regex);
    
    if (reti != 0) {
        strcpy(output, "Error: Input contains invalid characters");
        return output;
    }
    
    char temp1[MAX_OUTPUT_LENGTH] = {0};
    char temp2[MAX_OUTPUT_LENGTH] = {0};
    
    replaceChar(trimmedPtr, '&', "&amp;", temp1);
    replaceChar(temp1, '<', "&lt;", temp2);
    replaceChar(temp2, '>', "&gt;", temp1);
    replaceChar(temp1, '"', "&quot;", temp2);
    replaceChar(temp2, '\\'', "&#x27;", temp1);
    
    sprintf(output, "Valid text: %s", temp1);
    return output;
}

char* validateEmail(const char* email, char* output) {
    if (email == NULL || strlen(email) == 0) {
        strcpy(output, "Error: Email is null or empty");
        return output;
    }
    
    if (strlen(email) > 254) {
        strcpy(output, "Error: Email exceeds maximum length");
        return output;
    }
    
    regex_t regex;
    int reti = regcomp(&regex, "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]+$", REG_EXTENDED);
    reti = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    if (reti != 0) {
        strcpy(output, "Error: Invalid email format");
        return output;
    }
    
    sprintf(output, "Valid email: %s", email);
    return output;
}

char* validateAndProcessNumber(const char* input, char* output) {
    if (input == NULL || strlen(input) == 0) {
        strcpy(output, "Error: Number input is null or empty");
        return output;
    }
    
    regex_t regex;
    int reti = regcomp(&regex, "^-?[0-9]+$", REG_EXTENDED);
    reti = regexec(&regex, input, 0, NULL, 0);
    regfree(&regex);
    
    if (reti != 0) {
        strcpy(output, "Error: Invalid number format");
        return output;
    }
    
    long long number = atoll(input);
    long long squared = number * number;
    
    sprintf(output, "Number: %lld, Squared: %lld", number, squared);
    return output;
}

char* processOperation(const char* operationType, const char* input, char* output) {
    if (operationType == NULL || input == NULL) {
        strcpy(output, "Error: Operation type or input is null");
        return output;
    }
    
    char opType[50];
    strcpy(opType, operationType);
    for (int i = 0; opType[i]; i++) {
        opType[i] = tolower(opType[i]);
    }
    
    if (strcmp(opType, "text") == 0) {
        return validateAndSanitizeText(input, output);
    } else if (strcmp(opType, "email") == 0) {
        return validateEmail(input, output);
    } else if (strcmp(opType, "number") == 0) {
        return validateAndProcessNumber(input, output);
    } else {
        strcpy(output, "Error: Unknown operation type");
        return output;
    }
}

int main() {
    char output[MAX_OUTPUT_LENGTH];
    
    printf("=== Test Case 1: Valid Text ===\\n");
    processOperation("text", "Hello World 123!", output);
    printf("%s\\n", output);
    
    printf("\\n=== Test Case 2: Invalid Text with Special Characters ===\\n");
    processOperation("text", "<script>alert('XSS')</script>", output);
    printf("%s\\n", output);
    
    printf("\\n=== Test Case 3: Valid Email ===\\n");
    processOperation("email", "user@example.com", output);
    printf("%s\\n", output);
    
    printf("\\n=== Test Case 4: Valid Number ===\\n");
    processOperation("number", "42", output);
    printf("%s\\n", output);
    
    printf("\\n=== Test Case 5: Invalid Number ===\\n");
    processOperation("number", "abc123", output);
    printf("%s\\n", output);
    
    return 0;
}
