#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <regex.h>
#include <errno.h>

#define PORT 8082
#define BUFFER_SIZE 4096
#define MAX_URL_SIZE 2048

// A regex that matches URLs where the host contains "example.com".
const char* ALLOWED_DOMAIN_REGEX = "^https?://[^/]*example\\.com.*";

// Fills out_url with the validated redirect URL.
void getRedirectUrl(const char* target, char* out_url, size_t out_url_size) {
    if (target != NULL && target[0] != '\0') {
        regex_t regex;
        int reti = regcomp(&regex, ALLOWED_DOMAIN_REGEX, REG_EXTENDED | REG_NOSUB);
        if (reti) {
            fprintf(stderr, "Could not compile regex\n");
            strncpy(out_url, "/", out_url_size);
            out_url[out_url_size - 1] = '\0';
            return;
        }

        reti = regexec(&regex, target, 0, NULL, 0);
        regfree(&regex);

        if (reti == 0) { // Match
            strncpy(out_url, target, out_url_size);
            out_url[out_url_size - 1] = '\0';
            return;
        }
    }
    // No match or no target
    strncpy(out_url, "/", out_url_size);
    out_url[out_url_size - 1] = '\0';
}

// A simple parser for the 'target' query parameter. Does not handle URL decoding.
char* find_target_param(const char* request) {
    const char* query_key = "target=";
    const char* target_ptr = strstr(request, query_key);
    if (!target_ptr) return NULL;

    const char* start = target_ptr + strlen(query_key);
    const char* end = start;
    while (*end != ' ' && *end != '&' && *end != '\0' && *end != '\r' && *end != '\n') {
        end++;
    }
    
    size_t len = end - start;
    if (len >= MAX_URL_SIZE) {
        fprintf(stderr, "Target URL is too long.\n");
        return NULL;
    }

    char* target_val = (char*) malloc(len + 1);
    if (target_val == NULL) {
        perror("malloc failed");
        return NULL;
    }

    memcpy(target_val, start, len);
    target_val[len] = '\0';
    return target_val;
}

void handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0) {
        perror("read failed");
        close(client_socket);
        return;
    }
    
    char* target = NULL;
    if (strncmp(buffer, "GET /", 5) == 0) {
        target = find_target_param(buffer);
    }
    
    char redirect_url[MAX_URL_SIZE];
    getRedirectUrl(target, redirect_url, sizeof(redirect_url));

    char response[BUFFER_SIZE];
    int response_len;

    if (strcmp(redirect_url, "/") == 0) {
        const char* home_page_body = "Welcome. Provide a valid 'target' parameter pointing to a URL "
                                     "at example.com to be redirected.";
        response_len = snprintf(response, sizeof(response),
                              "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s",
                              strlen(home_page_body), home_page_body);
    } else {
        response_len = snprintf(response, sizeof(response),
                              "HTTP/1.1 302 Found\r\nLocation: %s\r\n\r\n",
                              redirect_url);
    }

    if (response_len > 0 && (size_t)response_len < sizeof(response)) {
        if (write(client_socket, response, response_len) < 0) {
            perror("write failed");
        }
    }

    free(target); // free(NULL) is safe
    close(client_socket);
}

void runTests() {
    printf("\n--- Running 5 Test Cases ---\n");
    char url_buffer[MAX_URL_SIZE];

    const char* test1 = "http://example.com";
    getRedirectUrl(test1, url_buffer, sizeof(url_buffer));
    printf("1. (Valid) %s -> %s\n", test1, url_buffer);

    const char* test2 = "http://sub.example.com/some/path";
    getRedirectUrl(test2, url_buffer, sizeof(url_buffer));
    printf("2. (Valid) %s -> %s\n", test2, url_buffer);

    const char* test3 = "http://another-domain.com";
    getRedirectUrl(test3, url_buffer, sizeof(url_buffer));
    printf("3. (Invalid) %s -> %s\n", test3, url_buffer);

    const char* test4 = "http://evil.com/?q=example.com";
    getRedirectUrl(test4, url_buffer, sizeof(url_buffer));
    printf("4. (Invalid) %s -> %s\n", test4, url_buffer);
    
    getRedirectUrl(NULL, url_buffer, sizeof(url_buffer));
    printf("5. (Null target) NULL -> %s\n", url_buffer);

    printf("--- Test Cases Finished ---\n\n");
}

int main() {
    runTests();
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed"); exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt"); close(server_fd); exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed"); close(server_fd); exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen"); close(server_fd); exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    printf("Test with a browser, e.g., http://localhost:%d/?target=http://example.com\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            if (errno == EINTR) continue;
            perror("accept"); continue;
        }
        handle_connection(new_socket);
    }
    close(server_fd);
    return 0;
}