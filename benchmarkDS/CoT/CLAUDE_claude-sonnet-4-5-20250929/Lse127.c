
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERNAME_LENGTH 100
#define BUFFER_SIZE 512

void sanitizeInput(const char* input, char* output, size_t outputSize) {
    size_t j = 0;
    size_t i;
    
    for (i = 0; input[i] != '\\0' && j < outputSize - 10; i++) {
        switch (input[i]) {
            case '<':
                if (j + 4 < outputSize) {
                    strcpy(output + j, "&lt;");
                    j += 4;
                }
                break;
            case '>':
                if (j + 4 < outputSize) {
                    strcpy(output + j, "&gt;");
                    j += 4;
                }
                break;
            case '&':
                if (j + 5 < outputSize) {
                    strcpy(output + j, "&amp;");
                    j += 5;
                }
                break;
            case '"':
                if (j + 6 < outputSize) {
                    strcpy(output + j, "&quot;");
                    j += 6;
                }
                break;
            case '\\'':
                if (j + 6 < outputSize) {
                    strcpy(output + j, "&#x27;");
                    j += 6;
                }
                break;
            case '/':
                if (j + 6 < outputSize) {
                    strcpy(output + j, "&#x2F;");
                    j += 6;
                }
                break;
            default:
                if (input[i] >= 32 && input[i] <= 126) {
                    output[j++] = input[i];
                }
                break;
        }
    }
    output[j] = '\\0';
}

void httpHelloPage(FILE* request, FILE* response) {
    char username[BUFFER_SIZE];
    char sanitized[BUFFER_SIZE * 2];
    char responseMessage[BUFFER_SIZE * 3];
    
    memset(username, 0, sizeof(username));
    memset(sanitized, 0, sizeof(sanitized));
    
    if (fgets(username, sizeof(username) - 1, request) != NULL) {
        size_t len = strlen(username);
        if (len > 0 && username[len - 1] == '\\n') {
            username[len - 1] = '\\0';
            len--;
        }
        
        // Trim whitespace
        while (len > 0 && isspace((unsigned char)username[len - 1])) {
            username[--len] = '\\0';
        }
        
        size_t start = 0;
        while (username[start] && isspace((unsigned char)username[start])) {
            start++;
        }
        
        if (len == 0 || username[start] == '\\0') {
            strncpy(sanitized, "Guest", sizeof(sanitized) - 1);
        } else {
            if (len - start > MAX_USERNAME_LENGTH) {
                username[start + MAX_USERNAME_LENGTH] = '\\0';
            }
            sanitizeInput(username + start, sanitized, sizeof(sanitized));
        }
    } else {
        strncpy(sanitized, "Guest", sizeof(sanitized) - 1);
    }
    
    snprintf(responseMessage, sizeof(responseMessage), "Hello %s\\n", sanitized);
    fprintf(response, "%s", responseMessage);
    fflush(response);
}

int main() {
    const char* testCases[] = {
        "Alice\\n",
        "Bob<script>alert('xss')</script>\\n",
        "Charlie&Drop Table\\n",
        "\\n",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\\n"
    };
    
    for (int i = 0; i < 5; i++) {
        FILE* request = fmemopen((void*)testCases[i], strlen(testCases[i]), "r");
        FILE* response = fopen("/dev/null", "w");
        
        if (request && response) {
            printf("Test %d: ", i + 1);
            httpHelloPage(request, stdout);
            fclose(request);
            fclose(response);
        }
    }
    
    return 0;
}
