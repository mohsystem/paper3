#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

// For creating directories
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

#define IMAGE_DIR "images"
#define MAX_PATH_LEN 256

/**
 * @brief Simulates a web handler triggered by a URL.
 * 
 * This function parses the URL, extracts the filename, constructs the full path,
 * and checks if the file exists in the IMAGE_DIR. It prints the result to stdout.
 * 
 * @param url The request URL, e.g., "/images/cat.jpg".
 */
void handle_image_request(const char* url) {
    const char* prefix = "/images/";
    
    // Check if URL starts with the correct prefix
    if (strncmp(url, prefix, strlen(prefix)) != 0) {
        printf("HTTP 400 Bad Request: URL must start with %s\n", prefix);
        return;
    }

    const char* filename = url + strlen(prefix);

    // Security check for empty filename or directory traversal
    if (strlen(filename) == 0 || strstr(filename, "..") != NULL || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        printf("HTTP 400 Bad Request: Invalid filename.\n");
        return;
    }

    char full_path[MAX_PATH_LEN];
    // Securely build the path to prevent buffer overflows
    int ret = snprintf(full_path, MAX_PATH_LEN, "%s%c%s", IMAGE_DIR,
#ifdef _WIN32
        '\\',
#else
        '/',
#endif
        filename);

    if (ret < 0 || ret >= MAX_PATH_LEN) {
        printf("Error: Path length exceeded buffer size.\n");
        return;
    }

    struct stat buffer;
    // stat returns 0 on success (file exists)
    if (stat(full_path, &buffer) == 0) {
        // S_ISREG checks if it's a regular file (not a directory)
        if (S_ISREG(buffer.st_mode)) {
             printf("HTTP 200 OK: Sending file '%s' to the browser.\n", full_path);
        } else {
             printf("HTTP 404 Not Found: Path '%s' is not a regular file.\n", full_path);
        }
    } else {
        printf("HTTP 404 Not Found: File '%s' does not exist.\n", full_path);
    }
}

/**
 * @brief Sets up a dummy 'images' directory with some files for testing.
 */
void setup_dummy_files() {
    // Create the directory
    MKDIR(IMAGE_DIR);

    // Create a few empty files for testing
    const char* files[] = {"cat.jpg", "dog.png", "bird.gif"};
    for (int i = 0; i < 3; ++i) {
        char path[MAX_PATH_LEN];
        snprintf(path, MAX_PATH_LEN, "%s%c%s", IMAGE_DIR,
#ifdef _WIN32
        '\\',
#else
        '/',
#endif
        files[i]);
        FILE *fp = fopen(path, "w");
        if (fp) {
            fclose(fp);
        }
    }
}


int main() {
    // 1. Setup the environment
    setup_dummy_files();

    // 2. Run test cases
    printf("--- Running 5 Test Cases ---\n");
    
    // Test Case 1: Requesting an existing file
    printf("Test 1 (/images/cat.jpg):\n  ");
    handle_image_request("/images/cat.jpg");
    
    // Test Case 2: Requesting another existing file
    printf("Test 2 (/images/dog.png):\n  ");
    handle_image_request("/images/dog.png");

    // Test Case 3: Requesting a non-existent file
    printf("Test 3 (/images/lion.jpg):\n  ");
    handle_image_request("/images/lion.jpg");

    // Test Case 4: Attempting directory traversal (should be blocked)
    printf("Test 4 (/images/../somefile.txt):\n  ");
    handle_image_request("/images/../somefile.txt");

    // Test Case 5: Requesting a malformed URL
    printf("Test 5 (/other/cat.jpg):\n  ");
    handle_image_request("/other/cat.jpg");

    printf("\n--- Test Cases Finished ---\n");

    return 0;
}