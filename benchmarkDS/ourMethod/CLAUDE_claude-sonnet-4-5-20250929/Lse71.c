
/*
 * Secure File Upload Web Server in C
 * Implements HTTP POST file upload with security controls
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#define PORT 8080
#define MAX_BUFFER 65536
#define MAX_FILENAME 256
#define MAX_FILESIZE (10 * 1024 * 1024) /* 10MB limit */
#define UPLOAD_DIR "./uploads"

/* Security: Explicit memory clearing function to prevent optimization */
void secure_zero(void *ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) *p++ = 0;
}

/* Security: Validate filename - only alphanumeric, dash, underscore, dot */
int validate_filename(const char *filename) {
    size_t len;
    size_t i;
    
    if (filename == NULL) return 0;
    
    /* Security: Check length to prevent buffer overflow */
    len = strnlen(filename, MAX_FILENAME);
    if (len == 0 || len >= MAX_FILENAME) return 0;
    
    /* Security: Reject path traversal attempts */
    if (strstr(filename, "..") != NULL) return 0;
    if (strstr(filename, "/") != NULL) return 0;
    if (strstr(filename, "\\\") != NULL) return 0;\n    \n    /* Security: Allow list - only safe characters */\n    for (i = 0; i < len; i++) {\n        char c = filename[i];\n        if (!isalnum((unsigned char)c) && c != '.' && c != '-' && c != '_') {\n            return 0;\n        }\n    }\n    \n    /* Security: Reject dangerous extensions */\n    if (strstr(filename, ".exe") != NULL || \n        strstr(filename, ".sh") != NULL ||\n        strstr(filename, ".bat") != NULL ||\n        strstr(filename, ".cmd") != NULL ||\n        strstr(filename, ".com") != NULL) {\n        return 0;\n    }\n    \n    return 1;\n}\n\n/* Security: Safe path construction using openat */\nint safe_open_upload_file(const char *filename, int *dirfd) {\n    int fd;\n    struct stat st;\n    \n    /* Security: Open upload directory first */\n    *dirfd = open(UPLOAD_DIR, O_RDONLY | O_DIRECTORY | O_CLOEXEC);\n    if (*dirfd < 0) {\n        return -1;\n    }\n    \n    /* Security: Create file with restrictive permissions, O_NOFOLLOW prevents symlink attacks */\n    fd = openat(*dirfd, filename, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);\n    if (fd < 0) {\n        close(*dirfd);\n        return -1;\n    }\n    \n    /* Security: Validate opened file descriptor */\n    if (fstat(fd, &st) < 0) {\n        close(fd);\n        close(*dirfd);\n        return -1;\n    }\n    \n    /* Security: Ensure it's a regular file */\n    if (!S_ISREG(st.st_mode)) {\n        close(fd);\n        close(*dirfd);\n        unlinkat(*dirfd, filename, 0);\n        return -1;\n    }\n    \n    return fd;\n}\n\n/* Security: Extract filename from Content-Disposition header */\nint extract_filename(const char *header, char *filename, size_t filename_size) {\n    const char *start;\n    const char *end;\n    size_t len;\n    \n    if (header == NULL || filename == NULL || filename_size == 0) {\n        return 0;\n    }\n    \n    /* Security: Initialize output buffer */\n    memset(filename, 0, filename_size);\n    \n    /* Find filename= in header */\n    start = strstr(header, "filename=\"");
    if (start == NULL) {
        return 0;
    }
    start += 10; /* Skip 'filename="' */
    
    end = strchr(start, '"');
    if (end == NULL) {
        return 0;
    }
    
    /* Security: Check length before copy */
    len = end - start;
    if (len == 0 || len >= filename_size) {
        return 0;
    }
    
    /* Security: Use memcpy with explicit bound */
    memcpy(filename, start, len);
    filename[len] = '\0'; /* Ensure null termination */
    
    return 1;
}

/* Security: Handle file upload with size limit */
int handle_upload(int client_sock, const char *boundary, size_t content_length) {
    char *buffer = NULL;
    char filename[MAX_FILENAME];
    size_t bytes_read = 0;
    size_t file_bytes = 0;
    int upload_fd = -1;
    int dirfd = -1;
    int result = 0;
    char line[1024];
    int in_headers = 1;
    
    /* Security: Validate content length */
    if (content_length > MAX_FILESIZE || content_length == 0) {
        return 0;
    }
    
    /* Security: Allocate buffer with overflow check */
    if (content_length > SIZE_MAX - 1) {
        return 0;
    }
    buffer = calloc(1, content_length + 1);
    if (buffer == NULL) {
        return 0;
    }
    
    /* Read the multipart data */
    while (bytes_read < content_length) {
        ssize_t n = recv(client_sock, buffer + bytes_read, content_length - bytes_read, 0);
        if (n <= 0) {
            secure_zero(buffer, content_length);
            free(buffer);
            return 0;
        }
        bytes_read += n;
    }
    
    /* Security: Parse multipart headers to extract filename */
    memset(filename, 0, sizeof(filename));
    
    /* Find Content-Disposition line */
    char *pos = strstr(buffer, "Content-Disposition:");
    if (pos != NULL) {
        char *line_end = strstr(pos, "\\r\\n");
        if (line_end != NULL && (line_end - pos) < sizeof(line)) {
            memcpy(line, pos, line_end - pos);
            line[line_end - pos] = '\0';
            
            if (!extract_filename(line, filename, sizeof(filename))) {
                secure_zero(buffer, content_length);
                free(buffer);
                return 0;
            }
        }
    }
    
    /* Security: Validate extracted filename */
    if (!validate_filename(filename)) {
        secure_zero(buffer, content_length);
        free(buffer);
        return 0;
    }
    
    /* Find start of file data (after headers) */
    char *data_start = strstr(buffer, "\\r\\n\\r\\n");
    if (data_start == NULL) {
        secure_zero(buffer, content_length);
        free(buffer);
        return 0;
    }
    data_start += 4;
    
    /* Find end boundary */
    char end_boundary[256];
    int ret = snprintf(end_boundary, sizeof(end_boundary), "\\r\\n--%s", boundary);
    if (ret < 0 || ret >= sizeof(end_boundary)) {
        secure_zero(buffer, content_length);
        free(buffer);
        return 0;
    }
    
    char *data_end = strstr(data_start, end_boundary);
    if (data_end == NULL) {
        secure_zero(buffer, content_length);
        free(buffer);
        return 0;
    }
    
    /* Security: Calculate file size and validate */
    file_bytes = data_end - data_start;
    if (file_bytes > MAX_FILESIZE) {
        secure_zero(buffer, content_length);
        free(buffer);
        return 0;
    }
    
    /* Security: Open file safely using openat */
    upload_fd = safe_open_upload_file(filename, &dirfd);
    if (upload_fd < 0) {
        secure_zero(buffer, content_length);
        free(buffer);
        return 0;
    }
    
    /* Security: Write with explicit size */
    ssize_t written = write(upload_fd, data_start, file_bytes);
    if (written < 0 || (size_t)written != file_bytes) {
        close(upload_fd);
        close(dirfd);
        unlinkat(dirfd, filename, 0);
        secure_zero(buffer, content_length);
        free(buffer);
        return 0;
    }
    
    /* Security: Ensure data is written to disk */
    if (fsync(upload_fd) < 0) {
        close(upload_fd);
        close(dirfd);
        unlinkat(dirfd, filename, 0);
        secure_zero(buffer, content_length);
        free(buffer);
        return 0;
    }
    
    close(upload_fd);
    
    /* Security: Sync directory */
    fsync(dirfd);
    close(dirfd);
    
    result = 1;
    
    /* Security: Clear sensitive data */
    secure_zero(buffer, content_length);
    free(buffer);
    
    return result;
}

/* Security: Send response with constant format string */
void send_response(int client_sock, const char *status, const char *body) {
    char response[4096];
    int len;
    
    /* Security: Use snprintf with explicit bounds */
    len = snprintf(response, sizeof(response),
                   "HTTP/1.1 %s\\r\\n"
                   "Content-Type: text/html\\r\\n"
                   "Content-Length: %zu\\r\\n"
                   "Connection: close\\r\\n"
                   "\\r\\n%s",
                   status, strlen(body), body);
    
    if (len > 0 && len < sizeof(response)) {
        send(client_sock, response, len, 0);
    }
}

void handle_client(int client_sock) {
    char buffer[MAX_BUFFER];
    char method[16];
    char boundary[256];
    size_t content_length = 0;
    ssize_t received;
    
    /* Security: Initialize buffers */
    memset(buffer, 0, sizeof(buffer));
    memset(method, 0, sizeof(method));
    memset(boundary, 0, sizeof(boundary));
    
    /* Security: Read with explicit bound */
    received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        close(client_sock);
        return;
    }
    buffer[received] = '\0'; /* Security: Ensure null termination */
    
    /* Security: Parse method safely */
    if (sscanf(buffer, "%15s", method) != 1) {
        send_response(client_sock, "400 Bad Request", "<html><body>Bad Request</body></html>");
        close(client_sock);
        return;
    }
    
    /* Handle GET - serve upload form */
    if (strcmp(method, "GET") == 0) {
        const char *html = 
            "<html><body>"
            "<h1>Secure File Upload</h1>"
            "<form method='POST' enctype='multipart/form-data'>"
            "<input type='file' name='file'><br>"
            "<input type='submit' value='Upload'>"
            "</form></body></html>";
        send_response(client_sock, "200 OK", html);
        close(client_sock);
        return;
    }
    
    /* Handle POST - file upload */
    if (strcmp(method, "POST") == 0) {
        /* Security: Extract Content-Length */
        char *cl_header = strstr(buffer, "Content-Length:");
        if (cl_header != NULL) {
            /* Security: Use strtoul with validation */
            char *endptr = NULL;
            unsigned long val = strtoul(cl_header + 15, &endptr, 10);
            if (endptr != cl_header + 15 && val <= MAX_FILESIZE) {
                content_length = (size_t)val;
            }
        }
        
        /* Security: Extract boundary */
        char *boundary_header = strstr(buffer, "boundary=");
        if (boundary_header != NULL) {
            char *boundary_start = boundary_header + 9;
            char *boundary_end = strstr(boundary_start, "\\r\\n");
            if (boundary_end != NULL) {
                size_t boundary_len = boundary_end - boundary_start;
                if (boundary_len > 0 && boundary_len < sizeof(boundary)) {
                    memcpy(boundary, boundary_start, boundary_len);
                    boundary[boundary_len] = '\0';
                }
            }
        }
        
        /* Security: Validate required headers */
        if (content_length > 0 && content_length <= MAX_FILESIZE && strlen(boundary) > 0) {
            if (handle_upload(client_sock, boundary, content_length)) {
                send_response(client_sock, "200 OK", "<html><body>Upload successful!</body></html>");
            } else {
                send_response(client_sock, "400 Bad Request", "<html><body>Upload failed</body></html>");
            }
        } else {
            send_response(client_sock, "400 Bad Request", "<html><body>Invalid request</body></html>");
        }
        
        close(client_sock);
        return;
    }
    
    /* Security: Reject other methods */
    send_response(client_sock, "405 Method Not Allowed", "<html><body>Method Not Allowed</body></html>");
    close(client_sock);
}

int main(void) {
    int server_sock;
    int client_sock;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int opt = 1;
    
    /* Security: Create upload directory with restrictive permissions */
    mkdir(UPLOAD_DIR, 0700);
    
    /* Create socket */
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }
    
    /* Security: Set socket options */
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_sock);
        return EXIT_FAILURE;
    }
    
    /* Security: Initialize server address structure */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    /* Bind socket */
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_sock);
        return EXIT_FAILURE;
    }
    
    /* Listen for connections */
    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        close(server_sock);
        return EXIT_FAILURE;
    }
    
    printf("Server listening on port %d\\n", PORT);
    printf("Upload directory: %s\\n", UPLOAD_DIR);
    printf("Test with: curl -F 'file=@testfile.txt' http://localhost:%d/\\n", PORT);
    
    /* Accept and handle clients */
    while (1) {
        client_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_sock < 0) {
            perror("accept failed");
            continue;
        }
        
        handle_client(client_sock);
    }
    
    close(server_sock);
    return EXIT_SUCCESS;
}
