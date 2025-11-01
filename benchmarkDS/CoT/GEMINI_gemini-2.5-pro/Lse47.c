#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

#define MAX_ROUTES 10
#define MAX_URL_LEN 256
#define IMAGE_DIR "images"

// Define a function pointer type for request handlers
typedef char* (*handler_func)(const char*);

// A struct to hold route information
typedef struct {
    const char* url_prefix;
    handler_func handler;
} Route;

// Global array to store routes
Route routes[MAX_ROUTES];
int route_count = 0;

// Function to simulate a decorator by registering a route
void add_route(const char* prefix, handler_func handler) {
    if (route_count < MAX_ROUTES) {
        routes[route_count].url_prefix = prefix;
        routes[route_count].handler = handler;
        route_count++;
    }
}

// The handler function for serving images
char* serveImage(const char* filename) {
    // --- Security Check: Path Traversal ---
    // Check for ".." and path separators in the filename.
    if (strstr(filename, "..") != NULL || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        char* response = (char*)malloc(50);
        if (response) strcpy(response, "HTTP 400: Invalid filename.");
        return response;
    }

    // Construct the file path securely
    char file_path[MAX_URL_LEN];
    // snprintf prevents buffer overflows. +2 for '/' and null terminator.
    if (snprintf(file_path, sizeof(file_path), "%s/%s", IMAGE_DIR, filename) >= sizeof(file_path)) {
         char* response = (char*)malloc(60);
         if (response) strcpy(response, "HTTP 400: Filename is too long.");
         return response;
    }

    FILE* file = fopen(file_path, "r");
    if (file) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        const char* prefix = "HTTP 200: Serving content: ";
        size_t prefix_len = strlen(prefix);
        char* buffer = (char*)malloc(file_size + prefix_len + 1);
        if (buffer == NULL) {
            fclose(file);
            return NULL; // Out of memory
        }

        strcpy(buffer, prefix);
        fread(buffer + prefix_len, 1, file_size, file);
        buffer[file_size + prefix_len] = '\0';
        fclose(file);
        return buffer;
    } else {
        char* response = (char*)malloc(30);
        if (response) strcpy(response, "HTTP 404: File not found.");
        return response;
    }
}

// Dispatcher to find and execute the correct handler
char* dispatch(const char* url) {
    for (int i = 0; i < route_count; i++) {
        size_t prefix_len = strlen(routes[i].url_prefix);
        if (strncmp(url, routes[i].url_prefix, prefix_len) == 0) {
            const char* filename = url + prefix_len;
            return routes[i].handler(filename);
        }
    }
    char* response = (char*)malloc(30);
    if (response) strcpy(response, "HTTP 404: Route not found.");
    return response;
}

void run_test(int test_num, const char* description, const char* url) {
    printf("Test %d (%s): %s\n", test_num, description, url);
    char* response = dispatch(url);
    if (response) {
        printf("Response: %s\n\n", response);
        free(response); // IMPORTANT: Free the memory allocated by the handler
    } else {
        printf("Response: NULL (Out of memory)\n\n");
    }
}

int main() {
    // 1. Setup: Create dummy directory and files
    MKDIR(IMAGE_DIR);
    FILE* f1 = fopen("images/cat.jpg", "w");
    if (f1) {
        fprintf(f1, "This is a cat image");
        fclose(f1);
    }
    FILE* f2 = fopen("images/dog_1.png", "w");
    if (f2) {
        fprintf(f2, "This is a dog image");
        fclose(f2);
    }

    // Register our route, simulating the decorator
    add_route("/images/", serveImage);

    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Valid file
    run_test(1, "Valid", "/images/cat.jpg");
    
    // Test Case 2: File not found
    run_test(2, "Not Found", "/images/horse.gif");

    // Test Case 3: Path traversal attempt
    run_test(3, "Path Traversal", "/images/../secret.txt");

    // Test Case 4: Another path traversal with different separator
    run_test(4, "Path Traversal 2", "/images/..\\secret.txt");

    // Test Case 5: Invalid Route
    run_test(5, "Invalid Route", "/videos/movie.mp4");

    // Cleanup
    remove("images/cat.jpg");
    remove("images/dog_1.png");
#ifdef _WIN32
    _rmdir(IMAGE_DIR);
#else
    rmdir(IMAGE_DIR);
#endif

    return 0;
}