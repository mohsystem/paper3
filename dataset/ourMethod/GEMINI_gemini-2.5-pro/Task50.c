#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <limits.h>

// Use POSIX standard endian conversion functions
#if defined(__linux__)
#include <endian.h>
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define be16toh(x) OSSwapBigToHostInt16(x)
#define htobe16(x) OSSwapHostToBigInt16(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#endif

#define PORT 8080
#define UPLOAD_DIR "uploads"
#define BUFFER_SIZE 4096
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB
#define MAX_FILENAME_LEN 255
#define SERVER_BACKLOG 10

// Forward declarations for server functions
void handle_connection(int client_sock);
int sanitize_filename(const char* input, char* output, size_t out_size);
ssize_t read_from_socket(int fd, void* buf, size_t count);
ssize_t write_to_socket(int fd, const void* buf, size_t count);
void run_server(void);

// Forward declarations for test client functions
void run_tests(void);

/**
 * @brief Reads exactly 'count' bytes from a socket into a buffer.
 *
 * Handles short reads and interruptions.
 * @param fd The socket file descriptor.
 * @param buf The buffer to read into.
 * @param count The number of bytes to read.
 * @return The number of bytes read, or -1 on error. Returns a value less
 *         than count if EOF is reached.
 */
ssize_t read_from_socket(int fd, void* buf, size_t count) {
    size_t bytes_read = 0;
    while (bytes_read < count) {
        ssize_t result = recv(fd, (char*)buf + bytes_read, count - bytes_read, 0);
        if (result < 0) {
            if (errno == EINTR) continue; // Interrupted by signal, retry
            return -1; // Other error
        }
        if (result == 0) {
            // Connection closed by peer
            break;
        }
        bytes_read += result;
    }
    return bytes_read;
}

/**
 * @brief Writes exactly 'count' bytes to a socket from a buffer.
 *
 * Handles short writes and interruptions.
 * @param fd The socket file descriptor.
 * @param buf The buffer to write from.
 * @param count The number of bytes to write.
 * @return The number of bytes written, or -1 on error.
 */
ssize_t write_to_socket(int fd, const void* buf, size_t count) {
    size_t bytes_written = 0;
    while (bytes_written < count) {
        ssize_t result = send(fd, (const char*)buf + bytes_written, count - bytes_written, 0);
        if (result < 0) {
            if (errno == EINTR) continue; // Interrupted by signal, retry
            return -1; // Other error
        }
        bytes_written += result;
    }
    return bytes_written;
}

/**
 * @brief Sanitizes a filename to prevent path traversal and invalid characters.
 *
 * Rejects filenames containing slashes, special names like "." or "..",
 * and characters outside a-zA-Z0-9._- whitelist.
 * @param input The raw filename from the client.
 * @param output The buffer to store the sanitized filename.
 * @param out_size The size of the output buffer.
 * @return 0 on success, -1 on failure (invalid filename).
 */
int sanitize_filename(const char* input, char* output, size_t out_size) {
    if (input == NULL || output == NULL || out_size == 0) {
        return -1;
    }

    // Rule #7: Disallow path traversal characters
    if (strchr(input, '/') != NULL || strchr(input, '\\') != NULL) {
        return -1;
    }

    // Disallow special directory names
    if (strcmp(input, ".") == 0 || strcmp(input, "..") == 0) {
        return -1;
    }
    
    size_t len = strnlen(input, MAX_FILENAME_LEN + 1);
    if (len == 0 || len > MAX_FILENAME_LEN || len >= out_size) {
        return -1;
    }

    // Rule #3: Validate character set
    for (size_t i = 0; i < len; ++i) {
        if (!isalnum((unsigned char)input[i]) && input[i] != '.' && input[i] != '_' && input[i] != '-') {
            return -1; // Invalid character
        }
    }
    
    // Copy to output buffer
    strncpy(output, input, out_size - 1);
    output[out_size - 1] = '\0';
    
    // Final check for empty string after potential sanitization (not strictly needed with current checks but good practice)
    if (strlen(output) == 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Handles a single client connection for file upload.
 *
 * @param client_sock The socket for the connected client.
 */
void handle_connection(int client_sock) {
    int dir_fd = -1;
    int file_fd = -1;
    char* file_buffer = NULL;
    char raw_filename[MAX_FILENAME_LEN + 1] = {0};
    char sanitized_filename[MAX_FILENAME_LEN + 1] = {0};
    int error_occurred = 0;

    // Rule #7: Anchor all file operations to a trusted directory.
    dir_fd = open(UPLOAD_DIR, O_RDONLY | O_DIRECTORY);
    if (dir_fd < 0) {
        perror("Failed to open upload directory");
        error_occurred = 1;
        goto cleanup;
    }

    // 1. Read filename length
    uint16_t filename_len_n;
    if (read_from_socket(client_sock, &filename_len_n, sizeof(filename_len_n)) != sizeof(filename_len_n)) {
        fprintf(stderr, "Failed to read filename length\n");
        error_occurred = 1;
        goto cleanup;
    }
    uint16_t filename_len = be16toh(filename_len_n);

    if (filename_len == 0 || filename_len > MAX_FILENAME_LEN) {
        fprintf(stderr, "Invalid filename length: %u\n", filename_len);
        error_occurred = 1;
        goto cleanup;
    }

    // 2. Read filename
    if (read_from_socket(client_sock, raw_filename, filename_len) != filename_len) {
        fprintf(stderr, "Failed to read filename\n");
        error_occurred = 1;
        goto cleanup;
    }
    raw_filename[filename_len] = '\0';

    // 3. Sanitize filename
    if (sanitize_filename(raw_filename, sanitized_filename, sizeof(sanitized_filename)) != 0) {
        fprintf(stderr, "Invalid filename received: %s\n", raw_filename);
        error_occurred = 1;
        goto cleanup;
    }

    // 4. Read file size
    uint64_t file_size_n;
    if (read_from_socket(client_sock, &file_size_n, sizeof(file_size_n)) != sizeof(file_size_n)) {
        fprintf(stderr, "Failed to read file size\n");
        error_occurred = 1;
        goto cleanup;
    }
    uint64_t file_size = be64toh(file_size_n);

    // Rule #3, #5: Validate file size to prevent DoS
    if (file_size > MAX_FILE_SIZE) {
        fprintf(stderr, "File size %lu exceeds maximum of %ld\n", (unsigned long)file_size, (long)MAX_FILE_SIZE);
        error_occurred = 1;
        goto cleanup;
    }
    
    // Rule #8: Open file with O_EXCL to prevent TOCTOU and overwrites.
    // Use openat() to ensure the file is created inside the UPLOAD_DIR.
    file_fd = openat(dir_fd, sanitized_filename, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (file_fd < 0) {
        perror("Failed to create file");
        error_occurred = 1;
        goto cleanup;
    }
    
    // 5. Read file content and write to disk
    file_buffer = (char*)malloc(BUFFER_SIZE);
    if (file_buffer == NULL) {
        perror("Failed to allocate buffer");
        error_occurred = 1;
        goto cleanup;
    }

    uint64_t bytes_received = 0;
    while (bytes_received < file_size) {
        size_t bytes_to_read = BUFFER_SIZE;
        if (file_size - bytes_received < BUFFER_SIZE) {
            bytes_to_read = file_size - bytes_received;
        }

        ssize_t read_len = recv(client_sock, file_buffer, bytes_to_read, 0);
        if (read_len <= 0) {
            perror("Socket read error or connection closed prematurely");
            error_occurred = 1;
            goto cleanup;
        }

        ssize_t write_len = write(file_fd, file_buffer, read_len);
        if (write_len != read_len) {
            perror("File write error");
            error_occurred = 1;
            goto cleanup;
        }
        bytes_received += read_len;
    }

    if (bytes_received != file_size) {
        fprintf(stderr, "File size mismatch: expected %lu, got %lu\n", (unsigned long)file_size, (unsigned long)bytes_received);
        error_occurred = 1;
        goto cleanup;
    }

cleanup:
    if (error_occurred) {
        fprintf(stderr, "Upload failed for %s.\n", sanitized_filename);
        write_to_socket(client_sock, "FAIL", 4);
        if (file_fd != -1) {
             // On failure, remove the partially created file.
            unlinkat(dir_fd, sanitized_filename, 0);
        }
    } else {
        printf("Successfully received file: %s (%lu bytes)\n", sanitized_filename, (unsigned long)file_size);
        write_to_socket(client_sock, "OK", 2);
    }

    if (file_buffer != NULL) free(file_buffer);
    if (file_fd != -1) close(file_fd);
    if (dir_fd != -1) close(dir_fd);
}


/**
 * @brief Sets up and runs the file upload server.
 */
void run_server(void) {
    // Create upload directory if it doesn't exist.
    if (mkdir(UPLOAD_DIR, 0755) == -1) {
        if (errno != EEXIST) {
            perror("Failed to create upload directory");
            exit(EXIT_FAILURE);
        }
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, SERVER_BACKLOG) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d. Uploads will be saved to '%s' directory.\n", PORT, UPLOAD_DIR);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue; // Continue to next connection
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_connection(client_sock);
        close(client_sock);
        printf("Connection closed.\n");
    }

    close(server_sock);
}

// ------------------- TEST CLIENT CODE -------------------

int connect_to_server(void) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Test client: socket creation failed");
        return -1;
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Test client: invalid address");
        close(sock);
        return -1;
    }
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Test client: connection failed");
        close(sock);
        return -1;
    }
    return sock;
}

int send_file_and_check_response(const char* local_path, const char* remote_name, uint64_t size_override, const char* expected_response) {
    printf("--- Running test: upload %s as %s ---\n", local_path, remote_name);
    
    int sock = connect_to_server();
    if (sock < 0) {
        fprintf(stderr, "Test failed: Could not connect to server.\n");
        return 0; // Failure
    }

    FILE* f = fopen(local_path, "rb");
    if (!f && size_override == 0) {
        perror("Test client: could not open local file");
        close(sock);
        return 0;
    }

    uint64_t file_size = 0;
    if (f) {
        fseek(f, 0, SEEK_END);
        file_size = ftell(f);
        fseek(f, 0, SEEK_SET);
    }
    if (size_override > 0) {
        file_size = size_override;
    }

    uint16_t name_len = strlen(remote_name);
    uint16_t name_len_n = htobe16(name_len);
    uint64_t file_size_n = htobe64(file_size);

    int success = 1;

    if (write_to_socket(sock, &name_len_n, sizeof(name_len_n)) < 0 ||
        write_to_socket(sock, remote_name, name_len) < 0 ||
        write_to_socket(sock, &file_size_n, sizeof(file_size_n)) < 0) {
        perror("Test client: failed to send headers");
        success = 0;
        goto test_cleanup;
    }

    if (f && file_size > 0 && size_override == 0) {
        char buffer[BUFFER_SIZE];
        size_t n;
        while ((n = fread(buffer, 1, BUFFER_SIZE, f)) > 0) {
            if (write_to_socket(sock, buffer, n) < 0) {
                perror("Test client: failed to send file content");
                success = 0;
                goto test_cleanup;
            }
        }
    }
    
    char response[5] = {0};
    if (read_from_socket(sock, response, strlen(expected_response)) != (ssize_t)strlen(expected_response)) {
         fprintf(stderr, "Test failed: Did not receive full response.\n");
         success = 0;
    } else if (strcmp(response, expected_response) != 0) {
        fprintf(stderr, "Test failed: Expected response '%s', got '%s'\n", expected_response, response);
        success = 0;
    } else {
        printf("Test passed: Got expected response '%s'.\n", expected_response);
    }


test_cleanup:
    if (f) fclose(f);
    close(sock);
    return success;
}

void run_tests() {
    printf("Running tests...\n");
    printf("NOTE: Ensure the server is running in another terminal.\n\n");
    
    // Create a dummy file for testing
    FILE* f = fopen("testfile.txt", "w");
    if (!f) { perror("Failed to create test file"); return; }
    fprintf(f, "This is a test file.");
    fclose(f);

    int passed = 0;
    int total = 5;

    // Test 1: Valid file upload
    passed += send_file_and_check_response("testfile.txt", "upload1.txt", 0, "OK");
    
    // Test 2: Path traversal attempt
    passed += send_file_and_check_response("testfile.txt", "../../etc/passwd", 0, "FAIL");

    // Test 3: File too large (by claiming it is)
    passed += send_file_and_check_response("testfile.txt", "largefile.txt", MAX_FILE_SIZE + 1, "FAIL");

    // Test 4: File already exists
    // First upload should succeed
    if (send_file_and_check_response("testfile.txt", "exists.txt", 0, "OK")) {
        // Second attempt should fail
        passed += send_file_and_check_response("testfile.txt", "exists.txt", 0, "FAIL");
    } else {
        fprintf(stderr, "Skipping 'already exists' test because first upload failed.\n");
        total--; // Don't count this test
    }
    
    // Test 5: Invalid filename characters
    passed += send_file_and_check_response("testfile.txt", "file;rm-rf.sh", 0, "FAIL");


    printf("\n--- Test Summary ---\n");
    printf("%d out of %d tests passed.\n", passed, total);

    // Cleanup test files
    remove("testfile.txt");
    char filepath[PATH_MAX];
    snprintf(filepath, sizeof(filepath), "%s/upload1.txt", UPLOAD_DIR);
    remove(filepath);
    snprintf(filepath, sizeof(filepath), "%s/exists.txt", UPLOAD_DIR);
    remove(filepath);
}


int main(int argc, char* argv[]) {
    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        run_tests();
    } else {
        run_server();
    }
    return 0;
}