#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROUTES 10
#define COMMAND_BUFFER_SIZE 256

// Define a function pointer type for our handlers
typedef void (*Handler)(const char*);

// A struct to represent a single route
typedef struct {
    const char* path;
    Handler handler;
} Route;

// An array of Route structs to act as our routing table
Route routes[MAX_ROUTES];
int route_count = 0;

/**
 * The handler function that performs the ping operation.
 * @param url The domain or IP address to ping.
 */
void ping(const char* url) {
    char command[COMMAND_BUFFER_SIZE];
    
    // Differentiate command for Windows and other OS using preprocessor directives
    #ifdef _WIN32
        // -n 1 sends 1 echo request on Windows
        snprintf(command, COMMAND_BUFFER_SIZE, "ping -n 1 %s > nul", url);
    #else
        // -c 1 sends 1 packet on Unix-like systems
        snprintf(command, COMMAND_BUFFER_SIZE, "ping -c 1 %s > /dev/null 2>&1", url);
    #endif

    printf("Executing: %s\n", command);

    // system() executes the command and returns its exit code
    int exit_code = system(command);

    if (exit_code == 0) {
        printf("URL %s is alive.\n", url);
    } else {
        printf("URL %s is not alive.\n", url);
    }
}

/**
 * Simulates a decorator by adding a function to the routing table.
 * @param path The URL path (e.g., "/ping").
 * @param handler_func The function to execute for this path.
 */
void route(const char* path, Handler handler_func) {
    if (route_count < MAX_ROUTES) {
        routes[route_count].path = path;
        routes[route_count].handler = handler_func;
        route_count++;
    } else {
        printf("Cannot add more routes. Routing table is full.\n");
    }
}

/**
 * Simulates handling a web request. It looks up the path and executes the associated function.
 * @param path The requested URL path.
 * @param url The parameter (target URL) for the handler function.
 */
void handleRequest(const char* path, const char* url) {
    for (int i = 0; i < route_count; ++i) {
        if (strcmp(routes[i].path, path) == 0) {
            routes[i].handler(url);
            return;
        }
    }
    printf("No handler found for path: %s\n", path);
}

int main() {
    // "Decorate" the ping function by binding it to the "/ping" URL path.
    route("/ping", ping);
    
    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: A known alive host
    handleRequest("/ping", "google.com");
    printf("\n");

    // Test Case 2: Localhost, should be alive
    handleRequest("/ping", "127.0.0.1");
    printf("\n");

    // Test Case 3: A non-existent domain, should not be alive
    handleRequest("/ping", "nonexistentdomain12345.com");
    printf("\n");
    
    // Test Case 4: An unroutable IP address
    handleRequest("/ping", "192.0.2.0"); // Reserved for documentation, should fail
    printf("\n");

    // Test Case 5: A request to a URL path that is not registered
    handleRequest("/status", "google.com");
    printf("\n");

    return 0;
}