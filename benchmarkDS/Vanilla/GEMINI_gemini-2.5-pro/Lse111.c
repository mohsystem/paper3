#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Executes the system's ping command for a given URL.
 *
 * @param url The URL or IP address to ping.
 * @return A dynamically allocated string containing the command's output.
 *         The caller is responsible for freeing this memory.
 */
char* ping(const char* url) {
    char command[256];
#ifdef _WIN32
    // For Windows, use -n for count
    snprintf(command, sizeof(command), "ping -n 2 %s 2>&1", url);
#else
    // For Linux, macOS, etc., use -c for count
    // "2>&1" redirects stderr to stdout to capture all output
    snprintf(command, sizeof(command), "ping -c 2 %s 2>&1", url);
#endif

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        const char *error_msg = "popen() failed!";
        char *result = malloc(strlen(error_msg) + 1);
        if (result) {
            strcpy(result, error_msg);
        }
        return result;
    }

    char buffer[128];
    // Start with a reasonable initial size
    size_t capacity = 512;
    char *result = (char*)malloc(capacity);
    if (!result) {
        pclose(pipe);
        return NULL; // Allocation failed
    }
    result[0] = '\0'; // Initialize as an empty string

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        // Check if we need more memory
        if (strlen(result) + strlen(buffer) + 1 > capacity) {
            capacity *= 2; // Double the capacity
            char *temp = (char*)realloc(result, capacity);
            if (!temp) {
                // Reallocation failed, clean up and return what we have
                free(result);
                pclose(pipe);
                return NULL;
            }
            result = temp;
        }
        strcat(result, buffer);
    }

    pclose(pipe);
    return result;
}

int main() {
    const char* testUrls[] = {
        "google.com",
        "localhost",
        "127.0.0.1",
        "nonexistentdomain12345.xyz",
        "8.8.8.8"
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("--- Pinging %s ---\n", testUrls[i]);
        char* result = ping(testUrls[i]);
        if (result) {
            printf("%s\n", result);
            free(result); // Free the dynamically allocated memory
        } else {
            printf("Failed to execute ping or allocate memory.\n");
        }
        printf("--- End of Ping for %s ---\n\n", testUrls[i]);
    }

    return 0;
}