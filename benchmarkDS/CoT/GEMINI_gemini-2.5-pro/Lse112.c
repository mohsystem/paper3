#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Validates if the string contains only characters suitable for a hostname.
 * This is a critical security function to prevent command injection.
 * @param hostname The string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int isValidHostname(const char* hostname) {
    if (hostname == NULL || *hostname == '\0') {
        return 0; // Empty or null string is invalid
    }
    for (int i = 0; hostname[i] != '\0'; i++) {
        if (!isalnum((unsigned char)hostname[i]) && hostname[i] != '.' && hostname[i] != '-') {
            return 0; // Invalid character found
        }
    }
    return 1;
}

/**
 * Executes the ping command and captures its output.
 * The caller is responsible for freeing the returned memory.
 * @param url The URL or IP to ping.
 * @return A dynamically allocated string with the command output, or an error message.
 */
char* pingUrl(const char* url) {
    if (!isValidHostname(url)) {
        const char* errorMsg = "Error: Invalid characters in URL. Possible command injection attempt blocked.";
        char* result = (char*)malloc(strlen(errorMsg) + 1);
        if (result) {
            strcpy(result, errorMsg);
        }
        return result;
    }

    char command[256];
    // Use snprintf for safe string formatting to prevent buffer overflows.
#ifdef _WIN32
    snprintf(command, sizeof(command), "ping -n 4 %s", url);
#else
    snprintf(command, sizeof(command), "ping -c 4 %s", url);
#endif
    
    FILE *pipe;
    // popen opens a shell, which is why input validation is critical.
    pipe = popen(command, "r");
    if (!pipe) {
        const char* errorMsg = "popen() failed!";
        char* result = (char*)malloc(strlen(errorMsg) + 1);
        if (result) {
            strcpy(result, errorMsg);
        }
        return result;
    }

    // Read the output into a dynamically growing buffer
    char buffer[128];
    size_t totalSize = 1;
    char *result = (char*)malloc(totalSize);
    if (!result) {
        pclose(pipe);
        return NULL;
    }
    result[0] = '\0';

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t len = strlen(buffer);
        char *newResult = (char*)realloc(result, totalSize + len);
        if (!newResult) {
            free(result);
            pclose(pipe);
            return NULL;
        }
        result = newResult;
        strcat(result, buffer);
        totalSize += len;
    }

    pclose(pipe);
    return result;
}

int main() {
    const char* testUrls[] = {
        "google.com",                  // Valid domain
        "8.8.8.8",                     // Valid IP address
        "nonexistent.domain.xyz",      // Should fail to resolve
        "127.0.0.1",                   // Localhost
        "google.com; ls"               // Command injection attempt
    };
    int numTests = sizeof(testUrls) / sizeof(testUrls[0]);

    for (int i = 0; i < numTests; ++i) {
        printf("--- Pinging: %s ---\n", testUrls[i]);
        char* result = pingUrl(testUrls[i]);
        if (result) {
            printf("%s\n", result);
            free(result); // Free the dynamically allocated memory
        } else {
            printf("Failed to execute command or allocate memory.\n");
        }
        printf("----------------------------------\n\n");
    }

    return 0;
}