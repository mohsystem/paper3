
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

void trim(char* str) {
    int start = 0;
    while (str[start] == ' ') start++;
    
    int end = strlen(str) - 1;
    while (end >= 0 && str[end] == ' ') end--;
    
    int i;
    for (i = 0; i <= end - start; i++) {
        str[i] = str[start + i];
    }
    str[i] = '\\0';
}

char* redirectUser(char* userInput, char* result) {
    char pageNames[][20] = {"home", "about", "contact", "products", "services"};
    char pageUrls[][50] = {
        "https://example.com/home",
        "https://example.com/about",
        "https://example.com/contact",
        "https://example.com/products",
        "https://example.com/services"
    };
    int numPages = 5;
    
    if (userInput == NULL || strlen(userInput) == 0) {
        strcpy(result, "Error: Invalid input");
        return result;
    }
    
    char normalizedInput[100];
    strcpy(normalizedInput, userInput);
    trim(normalizedInput);
    toLowerCase(normalizedInput);
    
    if (strlen(normalizedInput) == 0) {
        strcpy(result, "Error: Invalid input");
        return result;
    }
    
    for (int i = 0; i < numPages; i++) {
        if (strcmp(normalizedInput, pageNames[i]) == 0) {
            sprintf(result, "Redirecting to: %s", pageUrls[i]);
            return result;
        }
    }
    
    sprintf(result, "Error: Page not found. Redirecting to: %s", pageUrls[0]);
    return result;
}

int main() {
    char result[200];
    
    printf("=== Page Redirect System - Test Cases ===\\n\\n");
    
    // Test Case 1: Valid page - home
    char test1[] = "home";
    printf("Test 1 - Input: '%s'\\n", test1);
    printf("Result: %s\\n\\n", redirectUser(test1, result));
    
    // Test Case 2: Valid page - about
    char test2[] = "about";
    printf("Test 2 - Input: '%s'\\n", test2);
    printf("Result: %s\\n\\n", redirectUser(test2, result));
    
    // Test Case 3: Valid page with mixed case
    char test3[] = "PRODUCTS";
    printf("Test 3 - Input: '%s'\\n", test3);
    printf("Result: %s\\n\\n", redirectUser(test3, result));
    
    // Test Case 4: Invalid page
    char test4[] = "unknown";
    printf("Test 4 - Input: '%s'\\n", test4);
    printf("Result: %s\\n\\n", redirectUser(test4, result));
    
    // Test Case 5: Empty input
    char test5[] = "";
    printf("Test 5 - Input: '%s'\\n", test5);
    printf("Result: %s\\n\\n", redirectUser(test5, result));
    
    return 0;
}
