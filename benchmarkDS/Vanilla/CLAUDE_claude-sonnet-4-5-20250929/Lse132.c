
// C language - Web frameworks are not natively available
// This is a simplified demonstration of the template rendering logic
#include <stdio.h>
#include <string.h>

void hello(const char* username) {
    printf("<!DOCTYPE html>\\n");
    printf("<html>\\n");
    printf("<head><title>Hello</title></head>\\n");
    printf("<body>\\n");
    printf("    <h1>Hello, %s!</h1>\\n", username);
    printf("</body>\\n");
    printf("</html>\\n");
}

int main() {
    printf("=== Test Case 1: /hello/John ===\\n");
    hello("John");
    printf("\\n");
    
    printf("=== Test Case 2: /hello/Alice ===\\n");
    hello("Alice");
    printf("\\n");
    
    printf("=== Test Case 3: /hello/Bob ===\\n");
    hello("Bob");
    printf("\\n");
    
    printf("=== Test Case 4: /hello/Charlie ===\\n");
    hello("Charlie");
    printf("\\n");
    
    printf("=== Test Case 5: /hello/Diana ===\\n");
    hello("Diana");
    printf("\\n");
    
    // Note: Implementing a full web server in C would require libraries like:
    // libmicrohttpd, libevent, or similar HTTP server libraries
    
    return 0;
}
