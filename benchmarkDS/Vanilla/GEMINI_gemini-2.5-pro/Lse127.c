#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Reads a username from the request stream, constructs a "Hello [username]" message,
 *        and writes it to the response stream.
 * 
 * @param request A FILE stream to read the client's request from.
 * @param response A FILE stream to write the response to.
 */
void http_hello_page(FILE* request, FILE* response) {
    char username[256];
    if (fgets(username, sizeof(username), request) != NULL) {
        // Remove trailing newline character from fgets
        username[strcspn(username, "\r\n")] = 0;
        fprintf(response, "Hello %s\n", username);
    }
}

int main() {
    // Test cases
    const char* test_usernames[] = {"Alice", "Bob", "Charlie", "David Smith", "Eve123"};
    int num_tests = sizeof(test_usernames) / sizeof(test_usernames[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        const char* username = test_usernames[i];
        
        // Use temporary files to simulate streams
        FILE* request_file = tmpfile();
        FILE* response_file = tmpfile();

        if (request_file == NULL || response_file == NULL) {
            perror("Failed to create temporary files");
            return 1;
        }

        // Write username to the request "stream"
        fputs(username, request_file);
        fputs("\n", request_file);
        rewind(request_file); // Go back to the beginning of the file

        printf("Test Case %d:\n", i + 1);
        printf("Input Username: %s\n", username);

        // Call the function
        http_hello_page(request_file, response_file);

        // Read the result from the response "stream"
        rewind(response_file);
        char result[256];
        if (fgets(result, sizeof(result), response_file) != NULL) {
            // Remove trailing newline for clean printing
            result[strcspn(result, "\r\n")] = 0;
            printf("Generated Response: %s\n", result);
        } else {
            printf("Generated Response: (empty)\n");
        }
        
        printf("Expected Response: Hello %s\n", username);
        printf("-------------------------\n");

        // tmpfile() automatically removes the files on fclose()
        fclose(request_file);
        fclose(response_file);
    }

    return 0;
}