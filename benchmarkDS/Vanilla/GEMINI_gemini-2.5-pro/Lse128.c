#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Reads a username from a request stream and writes a greeting to a response stream.
 * Assumes the request body is in the format "username=[some_name]".
 * @param request The file stream containing the client request.
 * @param response The file stream to write the response to.
 */
void http_hello_page(FILE* request, FILE* response) {
    char buffer[256];
    const char* username = "guest"; // Default username

    // Read a line from the request stream
    if (fgets(buffer, sizeof(buffer), request) != NULL) {
        const char* prefix = "username=";
        size_t prefix_len = strlen(prefix);

        // Check if the line starts with "username="
        if (strncmp(buffer, prefix, prefix_len) == 0) {
            // Point to the start of the username value
            char* user_start = buffer + prefix_len;
            
            // Remove trailing newline character if it was read by fgets
            size_t user_len = strlen(user_start);
            if (user_len > 0 && user_start[user_len - 1] == '\n') {
                user_start[user_len - 1] = '\0';
            }
            
            username = user_start;
        }
    }

    fprintf(response, "hello %s", username);
    fflush(response);
}

// Helper function for running a test case
void test(const char* requestBody, const char* expectedResponse) {
    // Use temporary files to simulate file streams in a portable way
    FILE* request_stream = tmpfile();
    FILE* response_stream = tmpfile();

    if (request_stream == NULL || response_stream == NULL) {
        perror("Failed to create temporary files for testing");
        if (request_stream) fclose(request_stream);
        if (response_stream) fclose(response_stream);
        return;
    }

    // Write the request body to the request stream and rewind for reading
    fputs(requestBody, request_stream);
    rewind(request_stream);

    // Call the function
    http_hello_page(request_stream, response_stream);
    
    // Rewind the response stream to read the output
    rewind(response_stream);
    char actualResponse[256] = {0}; // Initialize buffer to all zeros
    fread(actualResponse, sizeof(char), sizeof(actualResponse) - 1, response_stream);

    // tmpfile() creates files that are automatically deleted on close
    fclose(request_stream);
    fclose(response_stream);

    // Print results
    printf("Request: \"%s\"\n", requestBody);
    printf("Expected: \"%s\"\n", expectedResponse);
    printf("Actual  : \"%s\"\n", actualResponse);
    printf("Result  : %s\n\n", strcmp(expectedResponse, actualResponse) == 0 ? "PASS" : "FAIL");
}


int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Standard username
    test("username=Alice", "hello Alice");

    // Test Case 2: Username with spaces
    test("username=  Bob  ", "hello   Bob  ");
    
    // Test Case 3: Empty username
    test("username=", "hello ");
    
    // Test Case 4: Malformed request (doesn't start with "username=")
    test("user=Charlie", "hello guest");
    
    // Test Case 5: Empty request
    test("", "hello guest");

    return 0;
}