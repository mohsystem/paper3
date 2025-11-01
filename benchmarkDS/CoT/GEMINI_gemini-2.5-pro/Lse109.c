#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Finds and prints a user's information from /etc/passwd.
 * This function safely reads the file line by line to prevent command injection
 * vulnerabilities that would arise from executing shell commands with user input.
 * It prints the result directly to avoid complex memory management.
 *
 * @param username The username to search for.
 */
void getUserInfo(const char* username) {
    // Basic validation to prevent invalid input
    if (username == NULL || strpbrk(username, "/;`$()|&><") != NULL) {
        printf("Invalid username provided.\n");
        return;
    }

    FILE *passwdFile = fopen("/etc/passwd", "r");
    if (passwdFile == NULL) {
        perror("Error opening /etc/passwd");
        return;
    }

    char line[1024];
    char searchPrefix[256];
    int found = 0;

    // Safely create the search prefix, preventing buffer overflow
    snprintf(searchPrefix, sizeof(searchPrefix), "%s:", username);
    size_t prefixLen = strlen(searchPrefix);

    while (fgets(line, sizeof(line), passwdFile) != NULL) {
        if (strncmp(line, searchPrefix, prefixLen) == 0) {
            printf("%s", line); // fgets includes the newline
            found = 1;
            break; // Found the user, no need to read further
        }
    }

    fclose(passwdFile);

    if (!found) {
        printf("User '%s' not found.\n", username);
    }
}

int main(int argc, char* argv[]) {
    // The original prompt asked for command-line argument processing.
    // If arguments are provided, we use them. Otherwise, we run test cases.
    if (argc > 1) {
        printf("Searching for user from command line: %s\n", argv[1]);
        printf("Result: ");
        getUserInfo(argv[1]);
    } else {
        // Running 5 test cases as requested.
        printf("--- Running Test Cases ---\n");
        const char* testUsers[] = {"root", "daemon", "nobody", "nonexistentuser123", "adm"};
        int numTests = sizeof(testUsers) / sizeof(testUsers[0]);

        for (int i = 0; i < numTests; ++i) {
            printf("\nSearching for user: %s\n", testUsers[i]);
            printf("Result: ");
            getUserInfo(testUsers[i]);
        }

        printf("\n--- Test Cases Finished ---\n");
        printf("You can also run this program with a command-line argument, e.g., ./your_program_name root\n");
    }

    return 0;
}