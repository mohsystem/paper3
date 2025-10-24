#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform-specific includes and setup
#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
    typedef SOCKET socket_t;
    #include <process.h> // for _beginthreadex
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>
    #define CLOSE_SOCKET close
    typedef int socket_t;
    const int INVALID_SOCKET = -1;
#endif

// Thread function to handle a client connection
#if defined(_WIN32) || defined(_WIN64)
unsigned __stdcall handle_client(void* client_socket_ptr) {
#else
void* handle_client(void* client_socket_ptr) {
#endif
    socket_t client_socket = *(socket_t*)client_socket_ptr;
    free(client_socket_ptr); // Free the dynamically allocated memory

    char client_ip[INET_ADDRSTRLEN];
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    getpeername(client_socket, (struct sockaddr*)&client_addr, &client_addr_size);
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    
    printf("Client connected: %s\n", client_ip);

    char buffer[4096];
    int bytes_received;

    // Echo messages back to client
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        printf("Received from %s: %.*s", client_ip, bytes_received, buffer);
        fflush(stdout);
        send(client_socket, buffer, bytes_received, 0);
    }

    if (bytes_received == 0) {
        printf("Client %s disconnected gracefully.\n", client_ip);
    } else {
        #if defined(_WIN32) || defined(_WIN64)
            fprintf(stderr, "recv failed with error: %d\n", WSAGetLastError());
        #else
            perror("recv failed");
        #endif
    }
    fflush(stdout);
    CLOSE_SOCKET(client_socket);
    return 0;
}

void start_server(int port) {
#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return;
    }
#endif

    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Bind failed.\n");
        CLOSE_SOCKET(server_socket);
        return;
    }

    if (listen(server_socket, 5) < 0) {
        fprintf(stderr, "Listen failed.\n");
        CLOSE_SOCKET(server_socket);
        return;
    }

    printf("Server started on port: %d\n", port);
    printf("Waiting for client connections...\n");

    while (1) {
        socket_t client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed.\n");
            continue;
        }

        // Allocate memory for the socket descriptor to pass to the thread
        socket_t* p_client_socket = malloc(sizeof(socket_t));
        if (!p_client_socket) {
            fprintf(stderr, "Failed to allocate memory for client socket.\n");
            CLOSE_SOCKET(client_socket);
            continue;
        }
        *p_client_socket = client_socket;

        // Create a new thread for each client
        #if defined(_WIN32) || defined(_WIN64)
            HANDLE thread_handle = (HANDLE)_beginthreadex(NULL, 0, &handle_client, p_client_socket, 0, NULL);
            if (thread_handle == 0) {
                fprintf(stderr, "_beginthreadex failed.\n");
                free(p_client_socket);
            } else {
                CloseHandle(thread_handle); // Detach the thread
            }
        #else
            pthread_t thread_id;
            if (pthread_create(&thread_id, NULL, handle_client, (void*)p_client_socket) != 0) {
                perror("pthread_create failed");
                free(p_client_socket);
            } else {
                pthread_detach(thread_id); // Detach the thread to run independently
            }
        #endif
    }

    CLOSE_SOCKET(server_socket);
#if defined(_WIN32) || defined(_WIN64)
    WSACleanup();
#endif
}

void main_with_tests() {
    // --- Test Cases ---
    // The server will run indefinitely. To test, you need a client program
    // or a tool like `telnet` or `netcat`.
    //
    // How to test (5 test cases):
    // 1. Compile and run this C program.
    // 2. Open a new terminal/command prompt.
    // 3. Test Case 1: Connect using netcat: `nc 127.0.0.1 12345` or telnet: `telnet 127.0.0.1 12345`
    // 4. Test Case 2: Type "Hello C Server" and press Enter. The server should echo it back.
    // 5. Test Case 3: Type "Testing POSIX threads" and press Enter.
    // 6. Test Case 4: Open another new terminal and connect again.
    //    Send a message from this new client. The server handles multiple clients concurrently.
    // 7. Test Case 5: Close one of the nc/telnet sessions (Ctrl+C or Ctrl+D).
    //    The other session should remain active.

    int port = 12345;
    start_server(port);
}

int main() {
    main_with_tests();
    return 0;
}