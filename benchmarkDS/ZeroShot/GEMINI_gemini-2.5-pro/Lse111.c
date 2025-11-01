#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

// Security: Validates the input string to ensure it only contains characters
// suitable for a hostname, preventing shell metacharacters.
bool isSecureHostname(const char* url) {
    if (url == NULL || *url == '\0') {
        return false;
    }
    size_t len = strlen(url);
    if (len > 255) {
        return false;
    }
    if (url[0] == '-' || url[len - 1] == '-' || url[0] == '.' || url[len - 1] == '.') {
        return false;
    }

    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)url[i]) && url[i] != '.' && url[i] != '-') {
            return false; // Character is not in the allowed whitelist
        }
    }
    return true;
}

/**
 * Executes the ping command for a given URL.
 * @param url The URL or IP address to ping.
 * @return A dynamically allocated string with the command output. The caller must free this string.
 *          Returns an allocated error string on validation failure. Returns NULL on system failure.
 */
char* ping(const char* url) {
    // Security: Sanitize and validate the input URL.
    if (!isSecureHostname(url)) {
        const char* errorMsg = "Error: Invalid or insecure hostname format provided.\n";
        char* result = (char*)malloc(strlen(errorMsg) + 1);
        if (result) {
            strcpy(result, errorMsg);
        }
        return result;
    }

    char command[300]; // Buffer for the command string
#ifdef _WIN32
    // For Windows, -n specifies the number of echo requests.
    snprintf(command, sizeof(command), "ping -n 4 %s", url);
#else
    // For Linux/macOS, -c specifies the count.
    snprintf(command, sizeof(command), "ping -c 4 %s", url);
#endif

    // Security Note: popen executes the command via a shell (/bin/sh -c).
    // Because we have strictly validated the 'url' input against a whitelist
    // of characters, command injection is mitigated. However, using APIs like
    // fork/exec (on POSIX) or CreateProcess (on Windows) that do not invoke a shell
    // would be an even more secure approach.
    FILE* pipe = POPEN(command, "r");
    if (!pipe) {
        perror("popen failed");
        return NULL;
    }

    size_t buffer_size = 1024;
    char* result = (char*)malloc(buffer_size);
    if (result == NULL) {
        PCLOSE(pipe);
        return NULL;
    }
    result[0] = '\0';
    size_t current_size = 0;

    char line[128];
    while (fgets(line, sizeof(line), pipe) != NULL) {
        size_t line_len = strlen(line);
        if (current_size + line_len + 1 > buffer_size) {
            buffer_size *= 2;
            char* new_result = (char*)realloc(result, buffer_size);
            if (new_result == NULL) {
                free(result);
                PCLOSE(pipe);
                return NULL;
            }
            result = new_result;
        }
        strcat(result, line);
        current_size += line_len;
    }

    PCLOSE(pipe);
    return result;
}

int main() {
    const char* testUrls[] = {
        "8.8.8.8",                  // Valid IP
        "google.com",               // Valid hostname
        "localhost",                // Valid hostname
        "invalid!hostname",         // Invalid characters
        "8.8.8.8; ls -la",          // Malicious command injection attempt
        NULL
    };

    for (int i = 0; testUrls[i] != NULL; i++) {
        printf("--- Pinging: %s ---\n", testUrls[i]);
        char* output = ping(testUrls[i]);
        if (output) {
            printf("%s", output);
            free(output); // Free the dynamically allocated memory
        } else {
            printf("Failed to execute ping command or allocate memory.\n");
        }
        printf("----------------------------------\n\n");
    }

    return 0;
}