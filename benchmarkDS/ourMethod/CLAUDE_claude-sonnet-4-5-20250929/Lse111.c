
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#define MAX_HOST_LENGTH 253
#define MAX_OUTPUT_SIZE 8192
#define MAX_COMMAND_SIZE 512

/* Whitelist pattern: only allow valid hostnames and IP addresses */
static const char* VALID_HOST_PATTERN = 
    "^([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)*[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?$|"
    "^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\."
    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";

/* Trim whitespace from string in-place */
static void trim(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == '\\0') {
        str[0] = '\\0';
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\\0';
}

/* Validate hostname/IP using regex whitelist */
static int validate_host(const char* host) {
    if (host == NULL || host[0] == '\\0') {
        return 0;
    }
    
    regex_t regex;
    int ret = regcomp(&regex, VALID_HOST_PATTERN, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        return 0;
    }
    
    ret = regexec(&regex, host, 0, NULL, 0);
    regfree(&regex);
    
    return (ret == 0);
}

/**
 * Simulates a web endpoint that pings a host.
 * SECURITY: Uses strict input validation to prevent command injection.
 * Only allows validated hostnames/IPs, no shell metacharacters.
 */
char* ping(const char* url) {
    /* Allocate output buffer - must be freed by caller */
    char* output = (char*)calloc(MAX_OUTPUT_SIZE, sizeof(char));
    if (output == NULL) {
        return NULL;
    }
    
    /* Input validation: reject NULL or empty */
    if (url == NULL || url[0] == '\\0') {
        strncpy(output, "Error: Invalid input", MAX_OUTPUT_SIZE - 1);
        return output;
    }
    
    /* Create mutable copy for trimming */
    char cleaned_url[MAX_HOST_LENGTH + 1];
    strncpy(cleaned_url, url, MAX_HOST_LENGTH);
    cleaned_url[MAX_HOST_LENGTH] = '\\0';
    trim(cleaned_url);
    
    /* Length validation */
    size_t len = strlen(cleaned_url);
    if (len == 0 || len > MAX_HOST_LENGTH) {
        strncpy(output, "Error: Invalid input", MAX_OUTPUT_SIZE - 1);
        return output;
    }
    
    /* Whitelist validation: prevents command injection */
    if (!validate_host(cleaned_url)) {
        strncpy(output, "Error: Invalid input", MAX_OUTPUT_SIZE - 1);
        return output;
    }
    
    /* Build command - validate no single quotes (defense in depth) */
    for (size_t i = 0; i < len; i++) {
        if (cleaned_url[i] == '\\'') {
            strncpy(output, "Error: Invalid input", MAX_OUTPUT_SIZE - 1);
            return output;
        }
    }
    
    /* Construct command with single quotes for shell safety */
    char command[MAX_COMMAND_SIZE];
    int cmd_len = snprintf(command, MAX_COMMAND_SIZE, "ping -c 4 '%s' 2>&1", cleaned_url);
    
    /* Check for truncation */
    if (cmd_len < 0 || cmd_len >= MAX_COMMAND_SIZE) {
        strncpy(output, "Error: Invalid input", MAX_OUTPUT_SIZE - 1);
        return output;
    }
    
    /* Execute command using popen (read-only) */
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        strncpy(output, "Error: Operation failed", MAX_OUTPUT_SIZE - 1);
        return output;
    }
    
    /* Read output with bounds checking */
    size_t total_read = 0;
    char buffer[128];
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t buf_len = strlen(buffer);
        
        /* Prevent buffer overflow */
        if (total_read + buf_len >= MAX_OUTPUT_SIZE - 1) {
            pclose(pipe);
            strncpy(output, "Error: Output too large", MAX_OUTPUT_SIZE - 1);
            return output;
        }
        
        strncat(output, buffer, MAX_OUTPUT_SIZE - total_read - 1);
        total_read += buf_len;
    }
    
    pclose(pipe);
    return output;
}

int main(void) {
    char* result = NULL;
    
    /* Test case 1: Valid hostname */
    printf("Test 1 - Valid hostname:\\n");
    result = ping("localhost");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    /* Test case 2: Valid IP address */
    printf("\\nTest 2 - Valid IP:\\n");
    result = ping("127.0.0.1");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    /* Test case 3: Command injection attempt (rejected) */
    printf("\\nTest 3 - Injection attempt:\\n");
    result = ping("localhost; cat /etc/passwd");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    /* Test case 4: Invalid input with special characters */
    printf("\\nTest 4 - Special characters:\\n");
    result = ping("host`whoami`");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    /* Test case 5: Empty input */
    printf("\\nTest 5 - Empty input:\\n");
    result = ping("");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    return 0;
}
