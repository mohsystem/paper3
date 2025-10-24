
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_INPUT_LENGTH 256
#define MAX_PAGES 10

typedef struct {
    char key[50];
    char value[100];
} PageMapping;

static PageMapping ALLOWED_PAGES[] = {
    {"home", "/home"},
    {"about", "/about"},
    {"contact", "/contact"},
    {"services", "/services"},
    {"profile", "/profile"}
};

static const int PAGES_COUNT = 5;

void sanitizeInput(const char* input, char* output) {
    int j = 0;
    for (int i = 0; input[i] != '\\0' && i < MAX_INPUT_LENGTH; i++) {
        if (isalnum(input[i])) {
            output[j++] = tolower(input[i]);
        }
    }
    output[j] = '\\0';
}

void redirectUser(const char* userInput, char* result) {
    if (userInput == NULL || strlen(userInput) == 0) {
        strcpy(result, "Error: Invalid input. Redirecting to default page: /home");
        return;
    }
    
    char sanitized[MAX_INPUT_LENGTH];
    sanitizeInput(userInput, sanitized);
    
    // Check against whitelist
    for (int i = 0; i < PAGES_COUNT; i++) {
        if (strcmp(sanitized, ALLOWED_PAGES[i].key) == 0) {
            sprintf(result, "Redirecting to: %s", ALLOWED_PAGES[i].value);
            return;
        }
    }
    
    strcpy(result, "Error: Page not found. Redirecting to default page: /home");
}

int main() {
    printf("=== User Redirection System (C) ===\\n\\n");
    
    // Test cases
    const char* testCases[] = {
        "home",
        "about",
        "contact",
        "invalid_page",
        "../../../etc/passwd"
    };
    
    int testCount = 5;
    char result[MAX_INPUT_LENGTH];
    
    for (int i = 0; i < testCount; i++) {
        printf("Input: \\"%s\\"\\n", testCases[i]);
        redirectUser(testCases[i], result);
        printf("Result: %s\\n\\n", result);
    }
    
    return 0;
}
