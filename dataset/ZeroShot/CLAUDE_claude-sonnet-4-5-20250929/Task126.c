
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
    while (str[start] == ' ' || str[start] == '\\t' || str[start] == '\\n' || str[start] == '\\r') {
        start++;
    }
    
    int end = strlen(str) - 1;
    while (end >= 0 && (str[end] == ' ' || str[end] == '\\t' || str[end] == '\\n' || str[end] == '\\r')) {
        end--;
    }
    
    int i;
    for (i = 0; i <= end - start; i++) {
        str[i] = str[start + i];
    }
    str[i] = '\\0';
}

char* redirectUser(char* userInput, char* result) {
    char sanitized[100];
    strcpy(sanitized, userInput);
    trim(sanitized);
    
    if (strlen(sanitized) == 0) {
        strcpy(result, "Invalid input. Redirecting to default page: /home");
        return result;
    }
    
    toLowerCase(sanitized);
    
    if (strcmp(sanitized, "home") == 0) {
        strcpy(result, "Redirecting to: /home");
    } else if (strcmp(sanitized, "about") == 0) {
        strcpy(result, "Redirecting to: /about");
    } else if (strcmp(sanitized, "contact") == 0) {
        strcpy(result, "Redirecting to: /contact");
    } else if (strcmp(sanitized, "services") == 0) {
        strcpy(result, "Redirecting to: /services");
    } else if (strcmp(sanitized, "products") == 0) {
        strcpy(result, "Redirecting to: /products");
    } else {
        strcpy(result, "Page not found. Redirecting to default page: /home");
    }
    
    return result;
}

int main() {
    char result[200];
    
    printf("Test Case 1:\\n");
    redirectUser("home", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 2:\\n");
    redirectUser("about", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 3:\\n");
    redirectUser("invalidpage", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 4:\\n");
    redirectUser("", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 5:\\n");
    redirectUser("PRODUCTS", result);
    printf("%s\\n\\n", result);
    
    return 0;
}
