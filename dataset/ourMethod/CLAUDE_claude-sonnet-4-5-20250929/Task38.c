
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_INPUT_LENGTH 1000
#define MAX_HTML_LENGTH 10000

/* Rule#1: Initialize all pointers to NULL */
/* HTML encode function to prevent XSS attacks */
/* Returns allocated string that must be freed by caller, or NULL on error */
char* htmlEncode(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t inputLen = strlen(input);
    /* Rule#4: Check bounds - worst case is every character becomes 6 chars (&quot;) */
    if (inputLen > SIZE_MAX / 7) {
        return NULL; /* Prevent overflow */
    }
    
    /* Rule#1: Check malloc return value */
    size_t maxOutputLen = inputLen * 7 + 1;
    char* output = (char*)malloc(maxOutputLen);
    if (output == NULL) {
        return NULL;
    }
    
    /* Rule#1: Initialize memory */
    memset(output, 0, maxOutputLen);
    
    size_t outPos = 0;
    
    /* Rule#4: Bounds checking in loop */
    for (size_t i = 0; i < inputLen && input[i] != '\\0'; i++) {
        /* Rule#4: Ensure we have space for worst case (6 chars + null) */
        if (outPos + 7 >= maxOutputLen) {
            free(output);
            return NULL;
        }
        
        switch (input[i]) {
            case '<':
                strcpy(output + outPos, "&lt;");
                outPos += 4;
                break;
            case '>':
                strcpy(output + outPos, "&gt;");
                outPos += 4;
                break;
            case '&':
                strcpy(output + outPos, "&amp;");
                outPos += 5;
                break;
            case '"':
                strcpy(output + outPos, "&quot;");
                outPos += 6;
                break;
            case '\\'':
                strcpy(output + outPos, "&#x27;");
                outPos += 6;
                break;
            default:
                output[outPos++] = input[i];
                break;
        }
    }
    
    /* Rule#1: Ensure null termination */
    output[outPos] = '\\0';
    
    return output;
}

/* Rule#3: Validate input for length and dangerous characters */
bool validateInput(const char* input, size_t maxLength) {
    if (input == NULL) {
        return false;
    }
    
    size_t len = strlen(input);
    
    /* Rule#3: Check length bounds */
    if (len > maxLength) {
        return false;
    }
    
    /* Rule#3: Check for control characters */
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)input[i];
        /* Allow printable characters, newlines, tabs, carriage returns */
        if (c < 32 && c != '\\n' && c != '\\r' && c != '\\t') {
            return false;
        }
    }
    
    return true;
}

/* Generate web page HTML */
/* Returns allocated string that must be freed by caller, or NULL on error */
char* generateWebPage(const char* userInput) {
    /* Rule#1: Check malloc return value */
    char* html = (char*)malloc(MAX_HTML_LENGTH);
    if (html == NULL) {
        return NULL;
    }
    
    /* Rule#1: Initialize memory */
    memset(html, 0, MAX_HTML_LENGTH);
    
    int written = 0;
    
    /* Rule#1: Use snprintf with bounds checking */
    written = snprintf(html, MAX_HTML_LENGTH,
        "<!DOCTYPE html>\\n"
        "<html lang=\\"en\\">\\n"
        "<head>\\n"
        "    <meta charset=\\"UTF-8\\">\\n"
        "    <meta http-equiv=\\"X-UA-Compatible\\" content=\\"IE=edge\\">\\n"
        "    <meta name=\\"viewport\\" content=\\"width=device-width, initial-scale=1.0\\">\\n"
        "    <meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'; script-src 'none'; object-src 'none';\\">\\n"
        "    <title>User Input Form</title>\\n"
        "    <style>\\n"
        "        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; padding: 20px; }\\n"
        "        .container { border: 1px solid #ccc; padding: 20px; border-radius: 5px; }\\n"
        "        .form-group { margin-bottom: 15px; }\\n"
        "        label { display: block; margin-bottom: 5px; font-weight: bold; }\\n"
        "        input[type=\\"text\\"] { width: 100%%; padding: 8px; box-sizing: border-box; }\\n"
        "        button { background-color: #4CAF50; color: white; padding: 10px 20px; border: none; cursor: pointer; }\\n"
        "        button:hover { background-color: #45a049; }\\n"
        "        .output { margin-top: 20px; padding: 15px; background-color: #f0f0f0; border-radius: 5px; }\\n"
        "    </style>\\n"
        "</head>\\n"
        "<body>\\n"
        "    <div class=\\"container\\">\\n"
        "        <h1>User Input Form</h1>\\n"
        "        <form method=\\"GET\\" action=\\"\\">\\n"
        "            <div class=\\"form-group\\">\\n"
        "                <label for=\\"userInput\\">Enter your message (max %d characters):</label>\\n"
        "                <input type=\\"text\\" id=\\"userInput\\" name=\\"userInput\\" maxlength=\\"%d\\" required>\\n"
        "            </div>\\n"
        "            <button type=\\"submit\\">Submit</button>\\n"
        "        </form>\\n",
        MAX_INPUT_LENGTH, MAX_INPUT_LENGTH);
    
    /* Rule#1: Check snprintf return value */
    if (written < 0 || written >= MAX_HTML_LENGTH) {
        free(html);
        return NULL;
    }
    
    /* Rule#3: Validate and sanitize user input before displaying */
    if (userInput != NULL && userInput[0] != '\\0') {
        if (validateInput(userInput, MAX_INPUT_LENGTH)) {
            /* Rule#2: HTML encode to prevent XSS */
            char* safeInput = htmlEncode(userInput);
            if (safeInput == NULL) {
                free(html);
                return NULL;
            }
            
            /* Rule#4: Calculate remaining space */
            size_t remaining = MAX_HTML_LENGTH - written;
            int additionalWritten = snprintf(html + written, remaining,
                "        <div class=\\"output\\">\\n"
                "            <h2>Your Input:</h2>\\n"
                "            <p>%s</p>\\n"
                "        </div>\\n",
                safeInput);
            
            /* Rule#1: Free allocated memory */
            free(safeInput);
            
            /* Rule#1: Check return value */
            if (additionalWritten < 0 || (size_t)additionalWritten >= remaining) {
                free(html);
                return NULL;
            }
            
            written += additionalWritten;
        } else {
            /* Rule#3: Reject invalid input */
            size_t remaining = MAX_HTML_LENGTH - written;
            int additionalWritten = snprintf(html + written, remaining,
                "        <div class=\\"output\\" style=\\"background-color: #ffcccc;\\">\\n"
                "            <h2>Error:</h2>\\n"
                "            <p>Invalid input. Please ensure your input is within the allowed length and contains no control characters.</p>\\n"
                "        </div>\\n");
            
            if (additionalWritten < 0 || (size_t)additionalWritten >= remaining) {
                free(html);
                return NULL;
            }
            
            written += additionalWritten;
        }
    }
    
    /* Close HTML tags */
    size_t remaining = MAX_HTML_LENGTH - written;
    int finalWritten = snprintf(html + written, remaining,
        "    </div>\\n"
        "</body>\\n"
        "</html>\\n");
    
    if (finalWritten < 0 || (size_t)finalWritten >= remaining) {
        free(html);
        return NULL;
    }
    
    return html;
}

int main(void) {
    char* result = NULL;
    
    /* Test case 1: Empty input */
    printf("=== Test Case 1: Empty Input ===\\n");
    result = generateWebPage("");
    if (result != NULL) {
        printf("Generated %zu bytes of HTML\\n", strlen(result));
        printf("%s\\n", strstr(result, "<form method=\\"GET\\"") != NULL ? "PASS" : "FAIL");
        free(result);
        result = NULL;
    }
    printf("\\n");
    
    /* Test case 2: Normal text input */
    printf("=== Test Case 2: Normal Text ===\\n");
    const char* input2 = "Hello, World!";
    result = generateWebPage(input2);
    if (result != NULL) {
        printf("Input: %s\\n", input2);
        printf("%s\\n", strstr(result, "Hello, World!") != NULL ? "PASS" : "FAIL");
        free(result);
        result = NULL;
    }
    printf("\\n");
    
    /* Test case 3: XSS attempt */
    printf("=== Test Case 3: XSS Prevention ===\\n");
    const char* input3 = "<script>alert('XSS')</script>";
    result = generateWebPage(input3);
    if (result != NULL) {
        printf("Input: %s\\n", input3);
        printf("%s\\n", strstr(result, "&lt;script&gt;") != NULL ? "PASS (XSS blocked)" : "FAIL");
        printf("%s\\n", strstr(result, "<script>") == NULL ? "PASS (no raw script)" : "FAIL");
        free(result);
        result = NULL;
    }
    printf("\\n");
    
    /* Test case 4: Special characters */
    printf("=== Test Case 4: Special Characters ===\\n");
    const char* input4 = "Tom's \\"great\\" idea & more";
    result = generateWebPage(input4);
    if (result != NULL) {
        printf("Input: %s\\n", input4);
        printf("%s\\n", strstr(result, "&quot;") != NULL ? "PASS (quotes encoded)" : "FAIL");
        printf("%s\\n", strstr(result, "&amp;") != NULL ? "PASS (ampersand encoded)" : "FAIL");
        free(result);
        result = NULL;
    }
    printf("\\n");
    
    /* Test case 5: Length validation */
    printf("=== Test Case 5: Length Validation ===\\n");
    /* Rule#1: Check malloc and initialize */
    char* input5 = (char*)malloc(1501);
    if (input5 != NULL) {
        memset(input5, 'A', 1500);
        input5[1500] = '\\0';
        
        result = generateWebPage(input5);
        if (result != NULL) {
            printf("Input length: %zu characters\\n", strlen(input5));
            printf("%s\\n", strstr(result, "Invalid input") != NULL ? "PASS (rejected)" : "FAIL");
            free(result);
            result = NULL;
        }
        
        /* Rule#1: Free allocated memory */
        free(input5);
    }
    printf("\\n");
    
    return 0;
}
