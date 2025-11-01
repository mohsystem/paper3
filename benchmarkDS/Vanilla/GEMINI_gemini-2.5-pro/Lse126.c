#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Simulates handling a request for the /hello page.
 * @param username The request body, expected to be a username.
 * @return A dynamically allocated string with the HTML response body.
 *         The caller is responsible for freeing this memory.
 */
char* http_hello_page(const char* username) {
    // The server constructs the response body (the HTML page).
    const char* part1 = "<html><body><h1>Hello, ";
    const char* part2 = "!</h1></body></html>";
    
    // Calculate the total length needed for the output buffer.
    size_t len = strlen(part1) + strlen(username) + strlen(part2) + 1; // +1 for null terminator

    // Allocate memory for the output buffer.
    char* outputbuf = (char*)malloc(len);
    if (outputbuf == NULL) {
        perror("Failed to allocate memory");
        return NULL; // Return NULL on allocation failure
    }

    // Safely construct the final string.
    strcpy(outputbuf, part1);
    strcat(outputbuf, username);
    strcat(outputbuf, part2);

    // In a real server, we would write this buffer to the response stream.
    // Here, we return it as a string to simulate the process.
    // The "closing" of the stream is handled by the caller.
    return outputbuf;
}

int main() {
    // 5 test cases
    const char* testUsernames[] = {"Alice", "Bob", "Charlie", "David", "Eve"};
    int num_tests = sizeof(testUsernames) / sizeof(testUsernames[0]);

    for (int i = 0; i < num_tests; ++i) {
        // The server reads the request body (the username) into the username variable.
        const char* username = testUsernames[i];

        printf("--- Test Case %d ---\n", i + 1);
        printf("Received username: %s\n", username);

        // The function http_hello_page is called when the server receives a request for the URL /hello.
        char* response = http_hello_page(username);
        
        if (response != NULL) {
            // The server writes the response body to the response stream.
            // (We print it to the console to simulate this)
            printf("Generated response: %s\n\n", response);

            // The server closes the response stream.
            // (Simulated by freeing the allocated memory for the response)
            free(response);
        } else {
            printf("Failed to generate response.\n\n");
        }
    }
    // The server is now ready to handle the next request (simulated by the loop).
    
    return 0;
}