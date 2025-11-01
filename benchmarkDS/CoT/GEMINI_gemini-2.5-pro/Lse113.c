#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Security: A helper function to validate the hostname.
// Allows only alphanumeric characters, hyphens, and dots.
// This is a crucial step to mitigate command injection when using system().
int isValidHostname(const char* host) {
    if (host == NULL) return 0;
    size_t len = strlen(host);
    if (len == 0 || len > 255) {
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)host[i]) && host[i] != '.' && host[i] != '-') {
            return 0;
        }
    }
    return 1;
}

/**
 * Pings a given host/URL to check if it is reachable.
 *
 * @param url The host or IP address to ping.
 * @return A const char* string indicating if the URL is "alive" or "not alive".
 */
const char* ping(const char* url) {
    if (!isValidHostname(url)) {
        return "URL is not alive (Invalid format).";
    }

    char command[512];
    
    // Differentiate ping command between Windows and other OS
    // snprintf is used for safe string construction to prevent buffer overflows.
#if defined(_WIN32) || defined(_WIN64)
    // -n 1 sends 1 echo request
    // > nul redirects standard output to null device
    snprintf(command, sizeof(command), "ping -n 1 %s > nul", url);
#else
    // -c 1 sends 1 packet
    // > /dev/null 2>&1 redirects stdout and stderr to null device
    snprintf(command, sizeof(command), "ping -c 1 %s > /dev/null 2>&1", url);
#endif

    // The output of the ping command is stored in "pong" (conceptually).
    // system() is used for simplicity but can be a security risk.
    // The input validation above is essential.
    int exitCode = system(command);

    // If the ping command returns 0, the URL is alive.
    if (exitCode == 0) {
        return "URL is alive.";
    } else {
        return "URL is not alive.";
    }
}

int main() {
    const char* testUrls[] = {
        "google.com",           // Test case 1: A valid, reachable domain
        "8.8.8.8",              // Test case 2: A valid, reachable IP address
        "localhost",            // Test case 3: Localhost
        "invalidurl12345.xyz",  // Test case 4: A non-existent domain
        "google.com; ls",       // Test case 5: A potential command injection attempt (should be rejected by validation)
        NULL
    };

    for (int i = 0; testUrls[i] != NULL; ++i) {
        const char* url = testUrls[i];
        const char* pong = ping(url);
        printf("Pinging %s: %s\n", url, pong);
    }

    return 0;
}