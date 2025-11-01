
// Note: C doesn't have decorators or web frameworks\n// This provides the core ping functionality with security measures\n// Compile: gcc -o Lse111 Lse111.c\n\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <ctype.h>\n#include <regex.h>\n\n#define MAX_URL_LENGTH 255\n#define MAX_OUTPUT_LENGTH 4096\n\nint validate_domain(const char* url) {\n    regex_t regex;\n    int ret;\n    \n    const char* pattern = "^([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)+[a-zA-Z]{2,}$";\n    \n    ret = regcomp(&regex, pattern, REG_EXTENDED);\n    if (ret) return 0;\n    \n    ret = regexec(&regex, url, 0, NULL, 0);\n    regfree(&regex);\n    \n    return !ret;\n}\n\nint validate_ip(const char* url) {\n    regex_t regex;\n    int ret;\n    \n    const char* pattern = "^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\."\n                         "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\."\n                         "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\."\n                         "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";\n    \n    ret = regcomp(&regex, pattern, REG_EXTENDED);\n    if (ret) return 0;\n    \n    ret = regexec(&regex, url, 0, NULL, 0);\n    regfree(&regex);\n    \n    return !ret;\n}\n\nchar* ping(const char* url) {\n    static char output[MAX_OUTPUT_LENGTH];\n    char command[512];\n    FILE* fp;\n    size_t len;\n    \n    // Validate input\n    if (url == NULL || strlen(url) == 0 || strlen(url) > MAX_URL_LENGTH) {\n        snprintf(output, MAX_OUTPUT_LENGTH, \n                "Invalid input: URL parameter is required and must be less than 255 characters");\n        return output;\n    }\n    \n    // Create sanitized copy\n    char sanitized_url[MAX_URL_LENGTH + 1];\n    strncpy(sanitized_url, url, MAX_URL_LENGTH);\n    sanitized_url[MAX_URL_LENGTH] = '\\0';\n    \n    // Remove whitespace\n    len = 0;\n    for (size_t i = 0; sanitized_url[i]; i++) {\n        if (!isspace((unsigned char)sanitized_url[i])) {\n            sanitized_url[len++] = sanitized_url[i];\n        }\n    }\n    sanitized_url[len] = '\\0';
    
    // Validate format
    if (!validate_domain(sanitized_url) && !validate_ip(sanitized_url)) {
        snprintf(output, MAX_OUTPUT_LENGTH, 
                "Invalid input: Must be a valid domain or IP address");
        return output;
    }
    
    // Build command
    #ifdef _WIN32
        snprintf(command, sizeof(command), "ping -n 4 %s", sanitized_url);
    #else
        snprintf(command, sizeof(command), "ping -c 4 %s", sanitized_url);
    #endif
    
    // Execute command
    fp = popen(command, "r");
    if (fp == NULL) {
        snprintf(output, MAX_OUTPUT_LENGTH, "Error: Failed to execute ping command");
        return output;
    }
    
    // Read output
    size_t pos = 0;
    while (fgets(output + pos, MAX_OUTPUT_LENGTH - pos, fp) != NULL) {
        pos = strlen(output);
        if (pos >= MAX_OUTPUT_LENGTH - 1) break;
    }
    
    pclose(fp);
    return output;
}

int main() {
    printf("Test 1 - Valid domain:\\n%s\\n\\n", ping("google.com"));
    printf("Test 2 - Valid IP:\\n%s\\n\\n", ping("8.8.8.8"));
    printf("Test 3 - Invalid input:\\n%s\\n\\n", ping("google.com; ls -la"));
    printf("Test 4 - Empty input:\\n%s\\n\\n", ping(""));
    printf("Test 5 - Invalid format:\\n%s\\n\\n", ping("not_a_valid_domain!"));
    
    return 0;
}
