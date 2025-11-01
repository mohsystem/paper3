#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_USERNAME_LEN 256
// Buffer size needs space for username, potential newline, and null terminator.
#define BUFFER_SIZE (MAX_USERNAME_LEN + 2)

/**
 * @brief Writes an HTML-escaped version of a string to a file stream.
 * This approach avoids dynamic memory allocation for the escaped string.
 * @param s The null-terminated string to escape and write.
 * @param stream The output file stream.
 */
void fputs_escaped(const char* s, FILE* stream) {
    if (!s) return;
    for (size_t i = 0; s[i] != '\0'; ++i) {
        switch (s[i]) {
            case '&':  fputs("&amp;", stream); break;
            case '\"': fputs("&quot;", stream); break;
            case '\'': fputs("&#x27;", stream); break;
            case '<':  fputs("&lt;", stream); break;
            case '>':  fputs("&gt;", stream); break;
            default:   fputc(s[i], stream); break;
        }
    }
}

/**
 * @brief Trims leading and trailing whitespace from a string in-place.
 * @param str The string to trim.
 * @return A pointer to the beginning of the trimmed string.
 */
char* trim_whitespace(char* str) {
    if (!str) return NULL;
    char *end;
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) // All spaces?
        return str;
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    // Write new null terminator
    *(end + 1) = '\0';
    return str;
}


/**
 * @brief Reads a username, constructs a "Hello" response, and writes it.
 * This function uses fgets for bounded reading to prevent buffer overflows
 * and sanitizes the username to prevent XSS.
 * @param request The input file stream containing the client request.
 * @param response The output file stream to send the response to.
 */
void http_hello_page(FILE* request, FILE* response) {
    char buffer[BUFFER_SIZE];
    const char* username = "Guest"; // Default username

    // Use fgets for a bounded read, preventing buffer overflows.
    if (fgets(buffer, sizeof(buffer), request) != NULL) {
        size_t len = strlen(buffer);
        // If the buffer is full and doesn't end with a newline, the line was too long.
        if (len == sizeof(buffer) - 1 && buffer[len-1] != '\n') {
            // Line is too long. Consume the rest of it from the input stream.
            int ch;
            while ((ch = fgetc(request)) != '\n' && ch != EOF);
            // We will use the default "Guest" username.
        } else {
            // Line is within limits. Remove trailing newline characters.
            buffer[strcspn(buffer, "\r\n")] = '\0';
            char* trimmed_username = trim_whitespace(buffer);
            if (trimmed_username[0] != '\0') {
                username = trimmed_username;
            }
        }
    }
    
    fputs("Hello ", response);
    fputs_escaped(username, response);
    fflush(response);
}

void run_test_case(const char* name, const char* input) {
    printf("--- %s ---\n", name);
    char printable_input[51];
    strncpy(printable_input, input, 50);
    printable_input[50] = '\0';
    if(strlen(input) > 50) strcat(printable_input, "...");
    printf("Input: \"%s\"\n", printable_input);
    
    FILE* request = tmpfile();
    if (!request) {
        perror("Failed to create temp file for request");
        return;
    }
    fputs(input, request);
    rewind(request);

    FILE* response = tmpfile();
    if (!response) {
        perror("Failed to create temp file for response");
        fclose(request);
        return;
    }

    http_hello_page(request, response);

    rewind(response);
    char response_buf[512] = {0};
    if (fgets(response_buf, sizeof(response_buf), response)) {
        printf("Output: \"%s\"\n", response_buf);
    } else {
        printf("Output: \"\"\n");
    }

    fclose(request);
    fclose(response);
    printf("\n");
}

int main(void) {
    run_test_case("Test Case 1: Normal", "Alice");
    run_test_case("Test Case 2: With Newline", "Bob\nnextline");
    run_test_case("Test Case 3: XSS Attempt", "<script>alert('xss')</script>");
    run_test_case("Test Case 4: Whitespace", "    ");
    
    char long_name[BUFFER_SIZE + 50];
    memset(long_name, 'A', sizeof(long_name) - 1);
    long_name[sizeof(long_name) - 1] = '\0';
    run_test_case("Test Case 5: Too Long", long_name);

    return 0;
}