
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define sleep(x) Sleep((x) * 1000)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#define PORT 8888
#define BUFFER_SIZE 1024

void* start_server(void* arg) {
    int port = *((int*)arg);
    
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\\n");
        return NULL;
    }
    #endif
    
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Could not create socket\\n");
        #ifdef _WIN32
        WSACleanup();
        #endif
        return NULL;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\\n");
        closesocket(server_socket);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return NULL;
    }
    
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\\n");
        closesocket(server_socket);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return NULL;
    }
    
    printf("Server started on port %d\\n", port);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket == INVALID_SOCKET) {
            continue;
        }
        
        printf("Client connected\\n");
        
        char buffer[BUFFER_SIZE];
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_read <= 0) {
                break;
            }
            
            printf("Received: %s", buffer);
            send(client_socket, buffer, bytes_read, 0);
            
            if (strstr(buffer, "QUIT") != NULL) {
                break;
            }
        }
        
        closesocket(client_socket);
        printf("Client disconnected\\n");
    }
    
    closesocket(server_socket);
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    return NULL;
}

char* send_message(int port, const char* message) {
    static char response[BUFFER_SIZE];
    memset(response, 0, BUFFER_SIZE);
    
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
    
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        #ifdef _WIN32
        WSACleanup();
        #endif
        strcpy(response, "Error: Could not create socket");
        return response;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);
    
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(client_socket);
        #ifdef _WIN32
        WSACleanup();
        #endif
        strcpy(response, "Error: Connection failed");
        return response;
    }
    
    char msg[BUFFER_SIZE];
    snprintf(msg, BUFFER_SIZE, "%s\\n", message);
    send(client_socket, msg, strlen(msg), 0);
    
    recv(client_socket, response, BUFFER_SIZE - 1, 0);
    
    closesocket(client_socket);
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    // Remove trailing newline
    size_t len = strlen(response);
    if (len > 0 && response[len - 1] == '\\n') {
        response[len - 1] = '\\0';
    }
    
    return response;
}

int main() {
    int port = PORT;
    
    // Start server in a separate thread
    #ifdef _WIN32
    HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start_server, &port, 0, NULL);
    #else
    pthread_t thread;
    pthread_create(&thread, NULL, start_server, &port);
    pthread_detach(thread);
    #endif
    
    // Wait for server to start
    sleep(1);
    
    // Test cases
    printf("\\n=== Running Test Cases ===\\n\\n");
    
    // Test 1: Simple message
    printf("Test 1: Simple message\\n");
    char* response1 = send_message(PORT, "Hello, Server!");
    printf("Sent: Hello, Server!\\n");
    printf("Echo: %s\\n\\n", response1);
    
    // Test 2: Numeric message
    printf("Test 2: Numeric message\\n");
    char* response2 = send_message(PORT, "12345");
    printf("Sent: 12345\\n");
    printf("Echo: %s\\n\\n", response2);
    
    // Test 3: Special characters
    printf("Test 3: Special characters\\n");
    char* response3 = send_message(PORT, "Test@#$%^&*()");
    printf("Sent: Test@#$%^&*()\\n");
    printf("Echo: %s\\n\\n", response3);
    
    // Test 4: Space message
    printf("Test 4: Space message\\n");
    char* response4 = send_message(PORT, " ");
    printf("Sent: ' '\\n");
    printf("Echo: '%s'\\n\\n", response4);
    
    // Test 5: Long message
    printf("Test 5: Long message\\n");
    const char* long_msg = "This is a longer message to test the echo functionality!";
    char* response5 = send_message(PORT, long_msg);
    printf("Sent: %s\\n", long_msg);
    printf("Echo: %s\\n\\n", response5);
    
    printf("=== All Tests Completed ===\\n");
    
    sleep(1);
    
    return 0;
}
