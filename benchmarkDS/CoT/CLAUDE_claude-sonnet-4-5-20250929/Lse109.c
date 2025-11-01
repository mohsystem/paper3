
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_USERNAME_LENGTH 32

int is_valid_username(const char* username) {
    if (username == NULL || strlen(username) == 0 || strlen(username) > MAX_USERNAME_LENGTH) {
        return 0;
    }
    
    for (int i = 0; username[i] != '\\0'; i++) {
        if (!isalnum(username[i]) && username[i] != '_' && username[i] != '-') {
            return 0;
        }
    }
    return 1;
}

char* get_user_info(const char* username) {
    static char result[MAX_LINE_LENGTH * 10];
    result[0] = '\\0';
    
    // Validate username to prevent injection attacks
    if (!is_valid_username(username)) {
        strcpy(result, "Invalid username format");
        return result;
    }
    
    FILE* file = fopen("/etc/passwd", "r");
    if (file == NULL) {
        strcpy(result, "File not found: /etc/passwd");
        return result;
    }
    
    char line[MAX_LINE_LENGTH];
    char search_str[MAX_USERNAME_LENGTH + 2];
    snprintf(search_str, sizeof(search_str), "%s:", username);
    
    int found = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        // Safely check if line starts with username followed by colon
        if (strncmp(line, search_str, strlen(search_str)) == 0) {
            // Remove newline if present
            line[strcspn(line, "\\n")] = '\\0';
            strcat(result, line);
            found = 1;
            break;
        }
    }
    
    fclose(file);
    
    if (!found) {
        strcpy(result, "User not found");
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* test_cases[] = {"root", "daemon", "nobody", "invalid@user", "testuser"};
    int num_tests = 5;
    
    if (argc > 1) {
        printf("%s\\n", get_user_info(argv[1]));
    } else {
        printf("Running test cases:\\n");
        for (int i = 0; i < num_tests; i++) {
            printf("\\n[Test: %s]\\n", test_cases[i]);
            printf("%s\\n", get_user_info(test_cases[i]));
        }
    }
    
    return 0;
}
