#include <stdio.h>
#include <string.h>

// Simple "web application" definitions
typedef const char* (*Handler)(const char* path);

typedef struct {
    const char* path;
    Handler handler;
} Route;

typedef struct {
    Route route;
} WebApp;

// Register a route
void route(WebApp* app, const char* path, Handler handler) {
    app->route.path = path;
    app->route.handler = handler;
}

// Dispatch a path to the handler
const char* dispatch(WebApp* app, const char* path) {
    if (app->route.path && strcmp(app->route.path, path) == 0) {
        return app->route.handler(path);
    }
    return "404 Not Found";
}

// Run the application (simulated)
void run_app(WebApp* app) {
    (void)app;
    puts("C WebApp running (simulated)");
}

// Function that accepts input and returns output
const char* root_handler(const char* path) {
    if (strcmp(path, "/") == 0) {
        return "Hello from C WebApp";
    }
    return "404 Not Found";
}

int main(void) {
    WebApp app = {0};                 // Create application object
    route(&app, "/", root_handler);   // Map URL to handler

    // 5 test cases
    const char* tests[5] = {"/", "/missing", "/", "/x", "/"};
    for (int i = 0; i < 5; ++i) {
        printf("GET %s -> %s\n", tests[i], dispatch(&app, tests[i]));
    }

    run_app(&app);                    // Run application (simulated)
    return 0;
}