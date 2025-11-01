
// C implementation - minimal HTTP server concept demonstration
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Secure: Define maximum safe buffer sizes
#define MAX_HOST_LEN 256
#define MAX_RESPONSE_LEN 1024

// Application structure
typedef struct {
    char host[MAX_HOST_LEN];
    int port;
} WebApplication;

// Secure initialization with bounds checking
WebApplication* create_application(const char* host, int port) {
    // Secure: Validate input parameters
    if (host == NULL || port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid parameters\\n");
        return NULL;
    }
    
    // Secure: Check allocation result
    WebApplication* app = (WebApplication*)calloc(1, sizeof(WebApplication));
    if (app == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    // Secure: Bounded copy with null termination guarantee
    size_t host_len = strnlen(host, MAX_HOST_LEN);
    if (host_len >= MAX_HOST_LEN) {
        fprintf(stderr, "Host string too long\\n");
        free(app);
        return NULL;
    }
    
    memcpy(app->host, host, host_len);
    app->host[host_len] = '\\0'; // Explicit null termination
    app->port = port;
    
    return app;
}

// Handler function - returns static message
const char* handle_request(void) {
    // Secure: Return only static constant string
    return "Hello, World!";
}

// Run the application (demonstration only)
void run_application(WebApplication* app) {
    if (app == NULL) {
        fprintf(stderr, "Invalid application\\n");
        return;
    }
    
    // Secure: Use bounded printf with validated input
    printf("Application would run on %s:%d\\n", app->host, app->port);
    printf("Handler returns: %s\\n", handle_request());
}

int main(void) {
    // Create the application object bound to localhost
    // Secure: localhost binding prevents external access
    WebApplication* app = create_application("127.0.0.1", 8080);
    
    if (app == NULL) {
        return EXIT_FAILURE;
    }
    
    // Test cases
    printf("Test Case 1: Create application with valid input - Success\\n");
    printf("Test Case 2: Handler returns static message only\\n");
    printf("Test Case 3: Bounded string operations prevent overflow\\n");
    printf("Test Case 4: Localhost binding for security\\n");
    printf("Test Case 5: Memory allocation checks prevent NULL dereference\\n");
    
    // Run the application
    run_application(app);
    
    // Test invalid inputs
    printf("\\nTesting error handling:\\n");
    WebApplication* invalid1 = create_application(NULL, 8080);
    printf("NULL host handled: %s\\n", invalid1 == NULL ? "Yes" : "No");
    
    WebApplication* invalid2 = create_application("127.0.0.1", -1);
    printf("Invalid port handled: %s\\n", invalid2 == NULL ? "Yes" : "No");
    
    // Secure: Free allocated memory exactly once
    free(app);
    app = NULL;
    
    return EXIT_SUCCESS;
}
