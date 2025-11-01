
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void urlDecode(const char* src, char* dest) {
    while (*src) {
        if (*src == '+') {
            *dest++ = ' ';
            src++;
        } else if (*src == '%') {
            int value;
            if (sscanf(src + 1, "%2x", &value) == 1) {
                *dest++ = (char)value;
                src += 3;
            } else {
                *dest++ = *src++;
            }
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\\0';
}

void sanitizeInput(const char* input, char* output, size_t outSize) {
    size_t j = 0;
    for (size_t i = 0; input[i] && j < outSize - 6; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&output[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&output[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&output[j], "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(&output[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&output[j], "&#x27;");
                j += 6;
                break;
            case '/':
                strcpy(&output[j], "&#x2F;");
                j += 6;
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\\0';
}

void httpHelloPage(FILE* request, FILE* response) {
    char line[2048];
    char username[512] = "";
    char decodedUsername[512] = "";
    char sanitized[2048] = "";
    
    // Read the request
    while (fgets(line, sizeof(line), request) != NULL) {
        // Remove newline
        line[strcspn(line, "\\r\\n")] = 0;
        
        if (strlen(line) == 0) {
            break;
        }
        
        // Look for username parameter
        char* pos = strstr(line, "username=");
        if (pos != NULL) {
            pos += 9;  // Skip "username="
            
            char* endPos = strchr(pos, '&');
            if (endPos == NULL) {
                endPos = strchr(pos, ' ');
            }
            
            if (endPos != NULL) {
                size_t len = endPos - pos;
                if (len < sizeof(username)) {
                    strncpy(username, pos, len);
                    username[len] = '\\0';
                }
            } else {
                strncpy(username, pos, sizeof(username) - 1);
                username[sizeof(username) - 1] = '\\0';
            }
            
            urlDecode(username, decodedUsername);
            break;
        }
    }
    
    // Sanitize username to prevent XSS attacks
    sanitizeInput(decodedUsername, sanitized, sizeof(sanitized));
    
    // Construct and send response
    fprintf(response, "HTTP/1.1 200 OK\\n");
    fprintf(response, "Content-Type: text/html; charset=UTF-8\\n");
    fprintf(response, "\\n");
    fprintf(response, "<html><body>\\n");
    fprintf(response, "<h1>hello %s</h1>\\n", sanitized);
    fprintf(response, "</body></html>\\n");
    fflush(response);
}

void testHttpHelloPage(const char* requestLine) {
    FILE* request = tmpfile();
    FILE* response = tmpfile();
    
    fprintf(request, "%s\\n\\n", requestLine);
    rewind(request);
    
    httpHelloPage(request, response);
    
    printf("Test: %s\\n", requestLine);
    rewind(response);
    
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), response) != NULL) {
        printf("%s", buffer);
    }
    printf("---\\n");
    
    fclose(request);
    fclose(response);
}

int main() {
    // Test case 1: Normal username
    testHttpHelloPage("GET /?username=John HTTP/1.1");
    
    // Test case 2: Username with spaces
    testHttpHelloPage("GET /?username=John+Doe HTTP/1.1");
    
    // Test case 3: Empty username
    testHttpHelloPage("GET /?username= HTTP/1.1");
    
    // Test case 4: Username with special characters (XSS attempt)
    testHttpHelloPage("GET /?username=<script>alert('xss')</script> HTTP/1.1");
    
    // Test case 5: Username with encoded characters
    testHttpHelloPage("GET /?username=Alice%20Smith HTTP/1.1");
    
    return 0;
}
