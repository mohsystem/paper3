
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Note: C does not have standard XML-RPC libraries.
 * This is a simplified demonstration of the concept.
 * For production use, consider using libxmlrpc or similar libraries.
 */

typedef struct {
    char method[256];
    int param1;
    int param2;
} RPCRequest;

/* Sanitize string input to prevent injection attacks */
void sanitizeInput(char* input, char* output, size_t maxLen) {
    size_t j = 0;
    size_t len = strlen(input);
    
    for (size_t i = 0; i < len && j < maxLen - 1; i++) {
        char c = input[i];
        /* Remove potentially dangerous characters */
        if (c != '<' && c != '>' && c != '&' && c != '"' && c != '\\'') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
}

/* Mathematical operations */
int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

double divide_numbers(int a, int b) {
    if (b == 0) {
        fprintf(stderr, "Error: Division by zero\\n");
        return 0.0;
    }
    return (double)a / (double)b;
}

void echo_message(const char* message, char* result, size_t maxLen) {
    if (message == NULL || strlen(message) == 0) {
        snprintf(result, maxLen, "Empty message");
        return;
    }
    
    char sanitized[256];
    char input_copy[256];
    strncpy(input_copy, message, sizeof(input_copy) - 1);
    input_copy[sizeof(input_copy) - 1] = '\\0';
    
    sanitizeInput(input_copy, sanitized, sizeof(sanitized));
    snprintf(result, maxLen, "Echo: %s", sanitized);
}

/* Build simple XML response */
void buildXMLResponse(const char* value, const char* type, char* output, size_t maxLen) {
    snprintf(output, maxLen,
        "<?xml version=\\"1.0\\"?>\\n"
        "<methodResponse>\\n"
        "  <params>\\n"
        "    <param>\\n"
        "      <value><%s>%s</%s></value>\\n"
        "    </param>\\n"
        "  </params>\\n"
        "</methodResponse>\\n",
        type, value, type);
}

/* Handle RPC request */
void handleRequest(const char* method, int param1, int param2, char* response, size_t maxLen) {
    char buffer[256];
    
    if (strcmp(method, "Math.add") == 0) {
        int result = add(param1, param2);
        snprintf(buffer, sizeof(buffer), "%d", result);
        buildXMLResponse(buffer, "int", response, maxLen);
    }
    else if (strcmp(method, "Math.subtract") == 0) {
        int result = subtract(param1, param2);
        snprintf(buffer, sizeof(buffer), "%d", result);
        buildXMLResponse(buffer, "int", response, maxLen);
    }
    else if (strcmp(method, "Math.multiply") == 0) {
        int result = multiply(param1, param2);
        snprintf(buffer, sizeof(buffer), "%d", result);
        buildXMLResponse(buffer, "int", response, maxLen);
    }
    else if (strcmp(method, "Math.divide") == 0) {
        double result = divide_numbers(param1, param2);
        snprintf(buffer, sizeof(buffer), "%.2f", result);
        buildXMLResponse(buffer, "double", response, maxLen);
    }
    else {
        buildXMLResponse("Unknown method", "string", response, maxLen);
    }
}

void runTests() {
    char result[512];
    
    printf("Starting XML-RPC Server Tests...\\n\\n");
    
    printf("=== Test Case 1: Addition ===\\n");
    printf("add(10, 5) = %d\\n", add(10, 5));
    
    printf("\\n=== Test Case 2: Subtraction ===\\n");
    printf("subtract(20, 8) = %d\\n", subtract(20, 8));
    
    printf("\\n=== Test Case 3: Multiplication ===\\n");
    printf("multiply(7, 6) = %d\\n", multiply(7, 6));
    
    printf("\\n=== Test Case 4: Division ===\\n");
    printf("divide(100, 4) = %.2f\\n", divide_numbers(100, 4));
    
    printf("\\n=== Test Case 5: Echo with sanitization ===\\n");
    echo_message("Hello World", result, sizeof(result));
    printf("echo('Hello World') = %s\\n", result);
    echo_message("<script>alert(1)</script>", result, sizeof(result));
    printf("echo('<script>alert(1)</script>') = %s\\n", result);
    
    printf("\\n=== Simulated XML-RPC Response ===\\n");
    char xmlResponse[1024];
    handleRequest("Math.add", 10, 5, xmlResponse, sizeof(xmlResponse));
    printf("%s\\n", xmlResponse);
}

int main() {
    runTests();
    return 0;
}
