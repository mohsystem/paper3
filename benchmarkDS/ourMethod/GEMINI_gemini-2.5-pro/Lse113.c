#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <sys/wait.h> // For WEXITSTATUS
#define POPEN popen
#define PCLOSE pclose
#endif

// Rule #1: Validate input to prevent OS command injection.
// Checks if all characters are either alphanumeric, a dot, or a hyphen.
int isValidHostname(const char* host) {
    if (host == NULL || *host == '\0') {
        return 0; // Invalid if null or empty
    }
    for (size_t i = 0; host[i] != '\0'; ++i) {
        if (!isalnum((unsigned char)host[i]) && host[i] != '.' && host[i] != '-') {
            return 0; // Disallowed character found
        }
    }
    return 1;
}

// The function ping() is called when the URL <url> ping is requested.
void ping(const char* url) {
    printf("--- Pinging %s ---\n", url);

    if (!isValidHostname(url)) {
        fprintf(stderr, "Invalid or potentially malicious URL/hostname format: %s\n", url);
        printf("Result: URL is not alive.\n");
        printf("------------------------------\n");
        return;
    }

    char command[256];
#ifdef _WIN32
    // snprintf is a safe way to format strings and prevent buffer overflows.
    int ret = snprintf(command, sizeof(command), "ping -n 1 -w 1000 %s", url);
#else
    int ret = snprintf(command, sizeof(command), "ping -c 1 -W 1 %s", url);
#endif

    if (ret < 0 || (size_t)ret >= sizeof(command)) {
        fprintf(stderr, "Error: Could not construct ping command (URL too long?).\n");
        return;
    }
    
    char pong[4096] = {0}; // Initialize buffer to all zeros
    char buffer[128];
    
    // Rule #2: Use potentially dangerous functions with caution.
    // popen() invokes a shell, so input validation (done above) is critical.
    FILE* pipe = POPEN(command, "r");
    if (!pipe) {
        perror("popen() failed");
        return;
    }

    // Rule #3: Ensure buffer operations are safe.
    // Read the output from the command safely, preventing buffer overflows.
    size_t pong_capacity = sizeof(pong) - 1; // Leave space for null terminator
    pong[0] = '\0';
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (strlen(pong) + strlen(buffer) < pong_capacity) {
            strcat(pong, buffer);
        } else {
            // Not enough space in `pong` buffer, append what fits and stop.
            strncat(pong, buffer, pong_capacity - strlen(pong));
            break; 
        }
    }

    int status = PCLOSE(pipe);

    printf("Command output (pong):\n");
    printf("%s", pong);

    int exit_code = -1;
    if (status == -1) {
        perror("pclose() failed");
    } else {
#ifdef _WIN32
        // On Windows, _pclose returns the exit code directly.
        exit_code = status;
#else
        // On POSIX, we need to use WEXITSTATUS.
        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        }
#endif
    }
    
    // If the ping command returns 0 the URL is alive.
    if (exit_code == 0) {
        printf("Result: URL '%s' is alive.\n", url);
    } else {
        // Else the URL is not alive.
        printf("Result: URL '%s' is not alive (exit code: %d).\n", url, exit_code);
    }
    printf("------------------------------\n");
}

int main(void) {
    printf("The decorator concept is simulated by calling the `ping` function for each test URL.\n\n");

    const char* testUrls[] = {
        "8.8.8.8",                  // A known alive IP
        "google.com",               // A known alive hostname
        "invalid-hostname-that-does-not-exist.com", // A likely dead hostname
        "127.0.0.1",                // Localhost, should be alive
        "google.com; rm -rf /"      // Malicious input to test validation
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    for (int i = 0; i < num_tests; ++i) {
        ping(testUrls[i]);
    }

    return 0;
}