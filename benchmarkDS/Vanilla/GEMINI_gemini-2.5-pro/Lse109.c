#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Executes a command and returns its output as a dynamically allocated string.
 * The caller is responsible for freeing the returned string.
 * @param username The username to search for.
 * @return A dynamically allocated string with the command output or a status message.
 */
char* getUserInfo(const char* username) {
    char command[256];
    // Use snprintf for safe string construction to prevent buffer overflows.
    // A more specific grep avoids partial matches and is anchored to the start of the line.
    snprintf(command, sizeof(command), "cat /etc/passwd | grep '^%s:'", username);

    FILE *pipe = popen(command, "r");
    if (!pipe) {
        perror("popen() failed");
        return NULL;
    }

    // Read the command's output into a dynamically growing string.
    char *output = NULL;
    size_t output_size = 0;
    char buffer[256];
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t len = strlen(buffer);
        char *new_output = (char*)realloc(output, output_size + len + 1);
        if (!new_output) {
            free(output);
            pclose(pipe);
            fprintf(stderr, "Memory allocation failed\n");
            return NULL;
        }
        output = new_output;
        strcpy(output + output_size, buffer);
        output_size += len;
    }
    
    pclose(pipe);

    // If output is still NULL, it means grep found nothing.
    if (output == NULL) {
        char not_found_msg[100];
        snprintf(not_found_msg, sizeof(not_found_msg), "User '%s' not found.\n", username);
        // strdup allocates memory and copies the string.
        output = strdup(not_found_msg);
    }
    
    return output;
}

int main(int argc, char* argv[]) {
    // argc is the argument count; argv[0] is the program name.
    if (argc > 1) {
        // If a command line argument is provided, use it.
        char* userInfo = getUserInfo(argv[1]);
        if (userInfo) {
            printf("%s", userInfo);
            free(userInfo); // Free the dynamically allocated memory.
        }
    } else {
        // Otherwise, run the built-in test cases.
        printf("No command line argument provided. Running built-in test cases:\n");
        printf("===============================================================\n");
        const char* testCases[] = {"root", "daemon", "bin", "sys", "nonexistentuser123"};
        int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
        
        for (int i = 0; i < numTestCases; ++i) {
            printf("\n--- Testing for user: '%s' ---\n", testCases[i]);
            char* userInfo = getUserInfo(testCases[i]);
            if (userInfo) {
                printf("%s", userInfo);
                free(userInfo); // Free memory for each result.
            }
        }
    }
    return 0;
}