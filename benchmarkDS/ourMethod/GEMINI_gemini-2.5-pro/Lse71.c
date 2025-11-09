#define _GNU_SOURCE // For memmem and strcasestr
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#define PORT 8080
#define UPLOAD_DIR "uploads"
#define MAX_REQUEST_SIZE (10 * 1024 * 1024) // 10 MB limit for the entire request
#define FILENAME_ALLOWED_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-"
#define MAX_FILENAME_LEN 255
#define READ_BUFFER_SIZE 8192

// Forward declarations
void handle_connection(int client_sock);
void send_response(int sock, const char* status, const char* content_type, const char* body);
int sanitize_filename(const char* input, char* output, size_t output_size);
void ensure_upload_dir_exists(void);


int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    ensure_upload_dir_exists();

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    printf("Uploading files to ./%s/ directory\n", UPLOAD_DIR);
    printf("--- Test Cases ---\n");
    printf("1. Valid upload:\n   curl -F 'file=@./test.txt' http://localhost:8080/\n");
    printf("2. Path traversal attempt (should be sanitized):\n   curl -F 'file=@./test.txt;filename=../../etc/passwd' http://localhost:8080/\n");
    printf("3. Invalid character in filename (should be sanitized):\n   curl -F 'file=@./test.txt;filename=\"bad<char>name.txt\"' http://localhost:8080/\n");
    printf("4. File already exists (should fail on second attempt):\n   curl -F 'file=@./test.txt;filename=existing.txt' http://localhost:8080/\n   curl -F 'file=@./test.txt;filename=existing.txt' http://localhost:8080/\n");
    printf("5. Non-POST request:\n   curl http://localhost:8080/\n");

    while (1) {
        if ((client_sock = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            continue; // Continue to next connection
        }
        handle_connection(client_sock);
    }

    close(server_fd);
    return 0;
}

void ensure_upload_dir_exists(void) {
    struct stat st = {0};
    if (stat(UPLOAD_DIR, &st) == -1) {
        if (mkdir(UPLOAD_DIR, 0700) != 0) {
            perror("Failed to create upload directory");
            exit(EXIT_FAILURE);
        }
    } else if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: '%s' exists but is not a directory.\n", UPLOAD_DIR);
        exit(EXIT_FAILURE);
    }
}


void handle_connection(int client_sock) {
    char *request_buf = NULL;
    char sanitized_filename[MAX_FILENAME_LEN + 1] = {0};
    char filepath[PATH_MAX] = {0};
    int fd = -1;
    const char *err_msg = "500 Internal Server Error";

    request_buf = malloc(MAX_REQUEST_SIZE);
    if (!request_buf) {
        perror("malloc");
        err_msg = "500 Internal Server Error";
        send_response(client_sock, err_msg, "text/plain", "Server memory allocation failed.");
        goto cleanup;
    }

    ssize_t total_read = 0;
    ssize_t bytes_read;
    while (total_read < MAX_REQUEST_SIZE && 
           (bytes_read = recv(client_sock, request_buf + total_read, READ_BUFFER_SIZE, 0)) > 0) {
        total_read += bytes_read;
    }

    if (bytes_read < 0) {
        perror("recv");
        err_msg = "500 Internal Server Error";
        send_response(client_sock, err_msg, "text/plain", "Failed to read from socket.");
        goto cleanup;
    }

    if (total_read >= MAX_REQUEST_SIZE) {
        err_msg = "413 Payload Too Large";
        send_response(client_sock, err_msg, "text/plain", "Request exceeds size limit.");
        goto cleanup;
    }

    char *headers_end = memmem(request_buf, total_read, "\r\n\r\n", 4);
    if (!headers_end) {
        err_msg = "400 Bad Request";
        send_response(client_sock, err_msg, "text/plain", "Malformed request: missing headers termination.");
        goto cleanup;
    }
    size_t headers_len = (headers_end - request_buf);
    char *body_start = headers_end + 4;
    size_t body_len = total_read - (headers_len + 4);

    // Temporarily null-terminate headers for string functions
    request_buf[headers_len] = '\0';

    if (strncmp(request_buf, "POST ", 5) != 0) {
        err_msg = "405 Method Not Allowed";
        send_response(client_sock, err_msg, "text/plain", "Only POST method is supported.");
        goto cleanup;
    }

    char *content_type_hdr = strcasestr(request_buf, "Content-Type: multipart/form-data;");
    if (!content_type_hdr) {
        err_msg = "415 Unsupported Media Type";
        send_response(client_sock, err_msg, "text/plain", "Content-Type must be multipart/form-data.");
        goto cleanup;
    }

    char *boundary_start = strcasestr(content_type_hdr, "boundary=");
    if (!boundary_start) {
        err_msg = "400 Bad Request";
        send_response(client_sock, err_msg, "text/plain", "Missing boundary in Content-Type header.");
        goto cleanup;
    }
    boundary_start += strlen("boundary=");
    
    char *boundary_end = strpbrk(boundary_start, "\r\n");
    if (!boundary_end) { boundary_end = request_buf + headers_len; }
    
    size_t boundary_len = boundary_end - boundary_start;
    char boundary[256];
    if (boundary_len == 0 || boundary_len >= sizeof(boundary) - 2) {
        err_msg = "400 Bad Request";
        send_response(client_sock, err_msg, "text/plain", "Invalid boundary string.");
        goto cleanup;
    }
    snprintf(boundary, sizeof(boundary), "--%.*s", (int)boundary_len, boundary_start);

    char *filename_tag = memmem(body_start, body_len, "filename=\"", 10);
    if (!filename_tag) {
        err_msg = "400 Bad Request";
        send_response(client_sock, err_msg, "text/plain", "Missing filename in multipart form.");
        goto cleanup;
    }
    filename_tag += 10;
    
    char *filename_end = memchr(filename_tag, '"', body_len - (filename_tag - body_start));
    if (!filename_end) {
        err_msg = "400 Bad Request";
        send_response(client_sock, err_msg, "text/plain", "Malformed filename in multipart form.");
        goto cleanup;
    }

    size_t raw_filename_len = filename_end - filename_tag;
    char raw_filename[MAX_FILENAME_LEN + 1];
    if (raw_filename_len >= sizeof(raw_filename)) {
        err_msg = "400 Bad Request";
        send_response(client_sock, err_msg, "text/plain", "Filename is too long.");
        goto cleanup;
    }
    memcpy(raw_filename, filename_tag, raw_filename_len);
    raw_filename[raw_filename_len] = '\0';

    if (!sanitize_filename(raw_filename, sanitized_filename, sizeof(sanitized_filename))) {
        err_msg = "400 Bad Request";
        send_response(client_sock, err_msg, "text/plain", "Invalid or empty filename after sanitization.");
        goto cleanup;
    }

    char *file_data_start = memmem(filename_end, body_len - (filename_end - body_start), "\r\n\r\n", 4);
    if (!file_data_start) {
        err_msg = "400 Bad Request";
        send_response(client_sock, err_msg, "text/plain", "Malformed multipart content.");
        goto cleanup;
    }
    file_data_start += 4;
    
    char end_boundary_marker[260];
    int end_boundary_len = snprintf(end_boundary_marker, sizeof(end_boundary_marker), "\r\n%s", boundary);
    if (end_boundary_len < 0 || (size_t)end_boundary_len >= sizeof(end_boundary_marker)) {
        err_msg = "500 Internal Server Error";
        send_response(client_sock, err_msg, "text/plain", "Boundary string processing error.");
        goto cleanup;
    }
    
    char *file_data_end = memmem(file_data_start, total_read - (file_data_start - request_buf), end_boundary_marker, end_boundary_len);
    if (!file_data_end) {
        err_msg = "400 Bad Request";
        send_response(client_sock, err_msg, "text/plain", "Could not find closing boundary.");
        goto cleanup;
    }
    
    size_t file_data_len = file_data_end - file_data_start;
    
    int path_len = snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, sanitized_filename);
    if (path_len < 0 || (size_t)path_len >= sizeof(filepath)) {
        err_msg = "500 Internal Server Error";
        send_response(client_sock, err_msg, "text/plain", "File path is too long.");
        goto cleanup;
    }

    fd = open(filepath, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        if (errno == EEXIST) {
            err_msg = "409 Conflict";
            send_response(client_sock, err_msg, "text/plain", "File already exists.");
        } else {
            perror("open");
            err_msg = "500 Internal Server Error";
            send_response(client_sock, err_msg, "text/plain", "Could not create file on server.");
        }
        goto cleanup;
    }
    
    ssize_t written = write(fd, file_data_start, file_data_len);
    if (written < 0 || (size_t)written != file_data_len) {
        perror("write");
        unlink(filepath); // Clean up partial file
        err_msg = "500 Internal Server Error";
        send_response(client_sock, err_msg, "text/plain", "Failed to write file to disk.");
        goto cleanup;
    }

    send_response(client_sock, "200 OK", "text/plain", "File uploaded successfully.");

cleanup:
    if (fd >= 0) close(fd);
    if (request_buf) free(request_buf);
    close(client_sock);
}


void send_response(int sock, const char* status, const char* content_type, const char* body) {
    char response_buf[1024];
    size_t body_len = body ? strlen(body) : 0;

    int len = snprintf(response_buf, sizeof(response_buf),
                       "HTTP/1.1 %s\r\n"
                       "Content-Type: %s\r\n"
                       "Content-Length: %zu\r\n"
                       "Connection: close\r\n\r\n"
                       "%s",
                       status, content_type, body_len, body ? body : "");

    if (len > 0 && (size_t)len < sizeof(response_buf)) {
        send(sock, response_buf, len, 0);
    }
}


int sanitize_filename(const char* input, char* output, size_t output_size) {
    if (!input || !output || output_size == 0) {
        return 0; // Invalid arguments
    }
    
    // Find the last path separator to get the basename
    const char *basename = strrchr(input, '/');
    basename = basename ? basename + 1 : input;
    
    const char *basename2 = strrchr(basename, '\\');
    basename = basename2 ? basename2 + 1 : basename;

    if (*basename == '\0') {
        output[0] = '\0';
        return 0; // Empty filename
    }

    size_t out_idx = 0;
    for (size_t i = 0; basename[i] != '\0' && out_idx < output_size - 1; ++i) {
        if (strchr(FILENAME_ALLOWED_CHARS, basename[i])) {
            output[out_idx++] = basename[i];
        }
    }
    output[out_idx] = '\0';

    return (out_idx > 0);
}