#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
    // On Windows, use _popen and _pclose
    #define popen _popen
    #define pclose _pclose
#else
    // For POSIX systems, for WEXITSTATUS
    #include <sys/wait.h>
#endif

/**
 * @brief Security: Validate hostname to contain only allowed characters.
 * @param url The string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int isValidHostname(const char* url) {
    if (url == NULL || *url == '\0') {
        return 0;
    }
    for (size_t i = 0; url[i] != '\0'; i++) {
        if (!isalnum((unsigned char)url[i]) && url[i] != '.' && url[i] != '-') {
            return 0; // Invalid character found
        }
    }
    return 1;
}

/**
 * @brief Pings a URL and captures the output.
 * @param url The URL or IP address to ping.
 * @param pong_buffer A buffer to store the command's output.
 * @param buffer_size The size of pong_buffer.
 * @return 0 for alive, 1 for not alive, -1 for error.
 */
int pingUrl(const char* url, char* pong_buffer, size_t buffer_size) {
    // Clear the output buffer initially
    if (buffer_size > 0) {
        pong_buffer[0] = '\0';
    }

    if (!isValidHostname(url)) {
        snprintf(pong_buffer, buffer_size, "Error: Invalid characters in URL.");
        return -1; // Error
    }

    char command[256];
#ifdef _WIN32
    snprintf(command, sizeof(command), "ping -n 4 %s", url);
#else
    snprintf(command, sizeof(command), "ping -c 4 %s", url);
#endif

    FILE* pipe = popen(command, "r");
    if (!pipe) {
        snprintf(pong_buffer, buffer_size, "Error: Failed to execute ping command.");
        return -1;
    }

    char line[128];
    size_t current_len = 0;
    while (fgets(line, sizeof(line), pipe) != NULL) {
        size_t line_len = strlen(line);
        if (current_len + line_len < buffer_size) {
            strcat(pong_buffer, line);
            current_len += line_len;
        } else {
            // Buffer is full, append a truncation message and stop
            const char* trunc_msg = "\n... (output truncated) ...";
            strncat(pong_buffer, trunc_msg, buffer_size - current_len - 1);
            break;
        }
    }

    int status = pclose(pipe);
    if (status == -1) {
        return -1; // Error in pclose
    }

#ifdef _WIN32
    // On Windows, the return value of pclose is the command's exit code.
    return (status == 0) ? 0 : 1;
#else
    // On POSIX, we need to use WIFEXITED and WEXITSTATUS macros.
    if (WIFEXITED(status)) {
        return (WEXITSTATUS(status) == 0) ? 0 : 1;
    }
    return -1; // Command did not terminate normally
#endif
}

int main() {
    const char* testUrls[] = {
        "8.8.8.8",
        "invalid-hostname-that-does-not-exist-12345.com",
        "127.0.0.1",
        "google.com",
        "google.com; ls -la" // Malicious input
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    for (int i = 0; i < num_tests; i++) {
        char pong_output[4096]; // A buffer for the command output
        const char* url = testUrls[i];

        printf("--- Testing URL: %s ---\n", url);
        int result = pingUrl(url, pong_output, sizeof(pong_output));
        
        const char* status_str;
        switch (result) {
            case 0:
                status_str = "is alive.";
                break;
            case 1:
                status_str = "is not alive.";
                break;
            default: // -1
                status_str = "could not be checked (error).";
                break;
        }
        
        printf("Status: The URL '%s' %s\n", url, status_str);
        printf("Output (pong):\n%s\n", pong_output);
        printf("---------------------------\n");
    }

    return 0;
}