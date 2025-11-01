// This C code is designed for a POSIX-compliant system (like Linux or macOS).
// It may require modifications to run on Windows.
// Compile with: gcc -o your_program_name your_source_file.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <regex.h>

/**
 * Executes the system's ping command for a given URL.
 * @param url The URL or IP address to ping.
 * @return A dynamically allocated string with the command output. The caller must free this memory.
 */
char* executePing(const char* url) {
    if (url == NULL) {
        return strdup("Error: URL is NULL.");
    }
    
    // Basic validation to prevent command injection using POSIX regex
    regex_t regex;
    if (regcomp(&regex, "^[a-zA-Z0-9.\\-]+$", REG_EXTENDED) != 0) {
        return strdup("Error: Could not compile regex.");
    }
    int reti = regexec(&regex, url, 0, NULL, 0);
    regfree(&regex);
    if (reti == REG_NOMATCH) {
        return strdup("Error: Invalid or potentially malicious URL format provided.");
    }

    char command[256];
    // Use -c 4 for Linux/macOS. For Windows, it's -n 4.
    snprintf(command, sizeof(command), "ping -c 4 %s", url);

    FILE* pipe = popen(command, "r");
    if (!pipe) {
        return strdup("popen() failed!");
    }

    size_t result_size = 2048;
    char* result = (char*)malloc(result_size);
    if (result == NULL) {
        pclose(pipe);
        return strdup("Memory allocation failed.");
    }
    result[0] = '\0';
    size_t current_len = 0;
    char buffer[128];

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t buffer_len = strlen(buffer);
        if (current_len + buffer_len + 1 > result_size) {
            result_size *= 2;
            char* new_result = (char*)realloc(result, result_size);
            if (new_result == NULL) {
                free(result);
                pclose(pipe);
                return strdup("Memory reallocation failed.");
            }
            result = new_result;
        }
        strcat(result, buffer);
        current_len += buffer_len;
    }

    pclose(pipe);
    return result;
}

void handle_connection(int client_socket) {
    char buffer[2048] = {0};
    read(client_socket, buffer, 2047);

    const char *ping_prefix = "GET /ping?url=";
    char* url_start = strstr(buffer, ping_prefix);
    char* response_body = NULL;

    if (url_start) {
        url_start += strlen(ping_prefix);
        char* url_end = strchr(url_start, ' ');
        if (url_end) {
            *url_end = '\0'; // Null-terminate the URL string
            response_body = executePing(url_start);
        }
    }
    
    if (response_body == NULL) {
        response_body = strdup("Please provide a URL. Example: /ping?url=google.com");
    }

    char http_header[512];
    snprintf(http_header, sizeof(http_header), 
             "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n", 
             strlen(response_body));

    write(client_socket, http_header, strlen(http_header));
    write(client_socket, response_body, strlen(response_body));

    free(response_body);
    close(client_socket);
}


void startServer(int port) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("\n--- Starting C Web Server on port %d ---\n", port);
    printf("To test the server, use curl:\n");
    printf("  curl 'http://localhost:%d/ping?url=google.com'\n", port);
    printf("Server listening... Press Ctrl+C to stop.\n");

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to next iteration
        }
        handle_connection(client_socket);
    }
}


void runTestCases() {
    printf("--- Running 5 Command-Line Test Cases ---\n");
    const char* testUrls[] = {
        "google.com",
        "127.0.0.1",
        "8.8.8.8",
        "nonexistent-domain-1234.com",
        "google.com; ls -la"
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("\n--- Pinging: %s ---\n", testUrls[i]);
        char* result = executePing(testUrls[i]);
        if (result) {
            printf("%s", result);
            free(result);
        }
    }
}

int main() {
    // 1. Run 5 test cases and print results to console
    runTestCases();

    // 2. Start the web server
    startServer(8080);

    return 0;
}