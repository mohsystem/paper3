
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_LENGTH 100000
#define MAX_OUTPUT_LENGTH 600000

// HTML entity encoding to prevent XSS attacks
// Returns allocated string that must be freed by caller, or NULL on error
char* htmlEncode(const char* input) {
    // Validate input pointer
    if (input == NULL) {
        return NULL;
    }
    
    // Validate input length to prevent excessive memory usage
    size_t inputLen = strlen(input);
    if (inputLen > MAX_INPUT_LENGTH) {
        char* errorMsg = (char*)malloc(20);
        if (errorMsg == NULL) {
            return NULL;
        }
        strncpy(errorMsg, "[Input too large]", 19);
        errorMsg[19] = '\\0';
        return errorMsg;
    }
    
    // Allocate output buffer (worst case: each char becomes 6-char entity)
    size_t maxOutputLen = inputLen * 6 + 1;
    if (maxOutputLen > MAX_OUTPUT_LENGTH) {
        maxOutputLen = MAX_OUTPUT_LENGTH;
    }
    
    char* output = (char*)calloc(maxOutputLen, sizeof(char));
    if (output == NULL) {
        return NULL;
    }
    
    size_t outPos = 0;
    // Process each character and encode dangerous HTML characters
    for (size_t i = 0; i < inputLen && outPos < maxOutputLen - 10; i++) {
        unsigned char c = (unsigned char)input[i];
        const char* entity = NULL;
        size_t entityLen = 0;
        
        switch (c) {
            case '&':  entity = "&amp;"; entityLen = 5; break;
            case '<':  entity = "&lt;"; entityLen = 4; break;
            case '>':  entity = "&gt;"; entityLen = 4; break;
            case '"':  entity = "&quot;"; entityLen = 6; break;
            case '\\'': entity = "&#x27;"; entityLen = 6; break;
            case '/':  entity = "&#x2F;"; entityLen = 6; break;
            default:
                // Only allow printable ASCII and common whitespace
                if ((c >= 32 && c <= 126) || c == '\\n' || c == '\\r' || c == '\\t') {
                    if (outPos < maxOutputLen - 1) {
                        output[outPos++] = c;
                    }
                } else {
                    // Replace non-printable with safe placeholder
                    entity = "&#xFFFD;";
                    entityLen = 8;
                }
                break;
        }
        
        // Copy entity if one was selected
        if (entity != NULL && outPos + entityLen < maxOutputLen) {
            memcpy(output + outPos, entity, entityLen);
            outPos += entityLen;
        }
    }
    
    // Ensure null termination
    output[outPos] = '\\0';
    return output;
}

// Generate complete HTML page with encoded user input
// Returns allocated string that must be freed by caller, or NULL on error
char* generateWebPage(const char* userInput) {
    // Validate input pointer
    if (userInput == NULL) {
        return NULL;
    }
    
    // Handle empty input
    if (strlen(userInput) == 0) {
        const char* emptyPage = 
            "<!DOCTYPE html>\\n<html lang=\\"en\\">\\n<head>\\n"
            "<meta charset=\\"UTF-8\\">\\n"
            "<meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'; script-src 'none'; object-src 'none';\\">\\n"
            "<title>User Input Display</title>\\n</head>\\n"
            "<body>\\n<h1>User Input:</h1>\\n<p>[No input provided]</p>\\n</body>\\n</html>";
        
        size_t len = strlen(emptyPage);
        char* page = (char*)malloc(len + 1);
        if (page == NULL) {
            return NULL;
        }
        memcpy(page, emptyPage, len + 1);
        return page;
    }
    
    // Sanitize input using HTML entity encoding
    char* safeInput = htmlEncode(userInput);
    if (safeInput == NULL) {
        return NULL;
    }
    
    // Calculate required buffer size
    const char* htmlTemplate1 = 
        "<!DOCTYPE html>\\n<html lang=\\"en\\">\\n<head>\\n"
        "<meta charset=\\"UTF-8\\">\\n"
        "<meta name=\\"viewport\\" content=\\"width=device-width, initial-scale=1.0\\">\\n"
        "<meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'; script-src 'none'; object-src 'none';\\">\\n"
        "<title>User Input Display</title>\\n</head>\\n<body>\\n"
        "<h1>User Input:</h1>\\n"
        "<div style=\\"border:1px solid #ccc; padding:10px; margin:10px;\\">\\n<pre>";
    
    const char* htmlTemplate2 = "</pre>\\n</div>\\n</body>\\n</html>";
    
    size_t totalLen = strlen(htmlTemplate1) + strlen(safeInput) + strlen(htmlTemplate2) + 1;
    
    // Allocate buffer for complete HTML page
    char* page = (char*)malloc(totalLen);
    if (page == NULL) {
        free(safeInput);
        return NULL;
    }
    
    // Build HTML page using safe string operations
    size_t pos = 0;
    size_t len1 = strlen(htmlTemplate1);
    memcpy(page + pos, htmlTemplate1, len1);
    pos += len1;
    
    size_t safeLen = strlen(safeInput);
    memcpy(page + pos, safeInput, safeLen);
    pos += safeLen;
    
    size_t len2 = strlen(htmlTemplate2);
    memcpy(page + pos, htmlTemplate2, len2);
    pos += len2;
    
    page[pos] = '\\0';
    
    // Clean up and return
    free(safeInput);
    return page;
}

int main(void) {
    // Test case 1: Normal text input
    {
        const char* input = "Hello, World!";
        char* page = generateWebPage(input);
        if (page != NULL) {
            printf("Test 1 - Normal text:\\n%s\\n\\n", page);
            free(page);
        }
    }
    
    // Test case 2: XSS attempt with script tags
    {
        const char* input = "<script>alert('XSS')</script>";
        char* page = generateWebPage(input);
        if (page != NULL) {
            printf("Test 2 - Script injection attempt:\\n%s\\n\\n", page);
            free(page);
        }
    }
    
    // Test case 3: HTML injection with image tag
    {
        const char* input = "<img src=x onerror=alert('XSS')>";
        char* page = generateWebPage(input);
        if (page != NULL) {
            printf("Test 3 - Image tag injection:\\n%s\\n\\n", page);
            free(page);
        }
    }
    
    // Test case 4: Special characters and quotes
    {
        const char* input = "Test \\"quotes\\" & 'apostrophes' <tags>";
        char* page = generateWebPage(input);
        if (page != NULL) {
            printf("Test 4 - Special characters:\\n%s\\n\\n", page);
            free(page);
        }
    }
    
    // Test case 5: Empty input
    {
        const char* input = "";
        char* page = generateWebPage(input);
        if (page != NULL) {
            printf("Test 5 - Empty input:\\n%s\\n\\n", page);
            free(page);
        }
    }
    
    return 0;
}
