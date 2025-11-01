
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void urlDecode(const char* src, char* dest) {
    while (*src) {
        if (*src == '%' && src[1] && src[2]) {
            int value;
            sscanf(src + 1, "%2x", &value);
            *dest++ = (char)value;
            src += 3;
        } else if (*src == '+') {
            *dest++ = ' ';
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\\0';
}

void httpHelloPage(FILE* request, FILE* response) {
    char username[256] = "";
    char line[1024];
    
    // Read the request to extract username
    while (fgets(line, sizeof(line), request) != NULL) {
        // Remove newline
        line[strcspn(line, "\\r\\n")] = 0;
        
        if (strlen(line) == 0) {
            break;
        }
        
        // Look for username parameter
        char* usernamePos = strstr(line, "username=");
        if (usernamePos != NULL) {
            char* start = usernamePos + 9; // Skip "username="
            char* end = start;
            while (*end && *end != '&' && *end != ' ' && *end != '\\r' && *end != '\\n') {
                end++;
            }
            
            char encoded[256];
            int len = end - start;
            strncpy(encoded, start, len);
            encoded[len] = '\\0';
            
            urlDecode(encoded, username);
        }
    }
    
    // Construct and write the response
    fprintf(response, "HTTP/1.1 200 OK\\n");
    fprintf(response, "Content-Type: text/html\\n");
    fprintf(response, "\\n");
    fprintf(response, "<html><body>\\n");
    fprintf(response, "<h1>hello %s</h1>\\n", username);
    fprintf(response, "</body></html>\\n");
    fflush(response);
}

void testCase(const char* requestStr, const char* expectedUser) {
    printf("Request: %s\\n", requestStr);
    
    // Create temporary files for testing
    FILE* request = tmpfile();
    FILE* response = tmpfile();
    
    fprintf(request, "%s", requestStr);
    rewind(request);
    
    httpHelloPage(request, response);
    
    rewind(response);
    printf("Response:\\n");
    char line[1024];
    while (fgets(line, sizeof(line), response) != NULL) {
        printf("%s", line);
    }
    printf("---\\n");
    
    fclose(request);
    fclose(response);
}

int main() {
    printf("Testing HTTP Hello Page Function:\\n\\n");
    
    // Test case 1
    testCase("GET /hello?username=John HTTP/1.1\\n", "John");
    
    // Test case 2
    testCase("GET /hello?username=Alice HTTP/1.1\\n", "Alice");
    
    // Test case 3
    testCase("GET /hello?username=Bob123 HTTP/1.1\\n", "Bob123");
    
    // Test case 4
    testCase("POST /hello HTTP/1.1\\nusername=Charlie\\n", "Charlie");
    
    // Test case 5
    testCase("GET /hello?username=Test%20User HTTP/1.1\\n", "Test User");
    
    return 0;
}
