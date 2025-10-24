#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define THREAD_T HANDLE
#define THREAD_FUNC_T DWORD WINAPI
#define close_socket closesocket
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define THREAD_T pthread_t
#define THREAD_FUNC_T void*
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#define close_socket close
#endif

#define PORT 12348
#define HOST "127.0.0.1"
#define USER_FILE "users_c.txt"
#define MAX_USERS 10
#define BUFFER_SIZE 1024

// --- SERVER COMPONENT ---

struct User {
    char username[50];
    char password[50];
};

struct User users[MAX_USERS];
int user_count = 0;
volatile bool server_should_stop = false;

void load_users() {
    FILE* file = fopen(USER_FILE, "r");
    if (!file) {
        perror("Could not open user file");
        return;
    }
    char line[101];
    user_count = 0;
    while (fgets(line, sizeof(line), file) && user_count < MAX_USERS) {
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0'; // Split string
            strcpy(users[user_count].username, line);
            char* newline = strchr(colon + 1, '\n');
            if(newline) *newline = '\0';
            strcpy(users[user_count].password, colon + 1);
            user_count++;
        }
    }
    printf("Users loaded from %s\n", USER_FILE);
    fclose(file);
}

void authenticate(const char* username, const char* password, char* response) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            if (strcmp(users[i].password, password) == 0) {
                strcpy(response, "LOGIN_SUCCESS");
                return;
            } else {
                strcpy(response, "LOGIN_FAIL Invalid password");
                return;
            }
        }
    }
    strcpy(response, "LOGIN_FAIL User not found");
}

void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int read_size;

    while ((read_size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        // Trim trailing newline characters
        buffer[strcspn(buffer, "\r\n")] = 0;
        printf("Server received: %s\n", buffer);

        char command[20], username[50], password[50];
        int items = sscanf(buffer, "%s %s %s", command, username, password);
        
        char response[100];
        if (items == 3 && strcmp(command, "LOGIN") == 0) {
            authenticate(username, password, response);
        } else if (strcmp(command, "SHUTDOWN") == 0) {
            server_should_stop = true;
            break;
        } else {
            strcpy(response, "ERROR Unknown command");
        }
        
        strcat(response, "\n");
        send(client_socket, response, strlen(response), 0);
    }
    close_socket(client_socket);
}

THREAD_FUNC_T server_main(void *arg) {
    #ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    #endif

    load_users();

    SOCKET server_fd;
    struct sockaddr_in server;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Could not create socket");
        return (THREAD_FUNC_T)1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed");
        return (THREAD_FUNC_T)1;
    }
    
    listen(server_fd, 3);
    printf("Server started on port %d\n", PORT);
    
    while(!server_should_stop) {
        SOCKET client_sock = accept(server_fd, NULL, NULL);
        if (client_sock == INVALID_SOCKET) {
             if(server_should_stop) break;
             perror("accept failed");
             continue;
        }
        handle_client(client_sock); // For simplicity, handle one at a time
    }

    close_socket(server_fd);
    #ifdef _WIN32
    WSACleanup();
    #endif
    printf("Server stopped.\n");
    return 0;
}

// --- CLIENT COMPONENT ---

void run_client() {
    printf("\n--- Starting Client Test Cases ---\n");
    #ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code : %d", WSAGetLastError());
        return;
    }
    #endif
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Could not create socket");
        return;
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(HOST);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return;
    }
    
    char server_reply[BUFFER_SIZE];
    
    auto send_and_receive = [&](const char* request) {
        char full_request[200];
        snprintf(full_request, sizeof(full_request), "%s\n", request);
        printf("Client sent: %s\n", request);
        if (send(sock, full_request, strlen(full_request), 0) < 0) {
            puts("Send failed");
            return;
        }
        int recv_size = recv(sock, server_reply, BUFFER_SIZE - 1, 0);
        if (recv_size > 0) {
            server_reply[recv_size] = '\0';
            server_reply[strcspn(server_reply, "\r\n")] = 0;
            printf("Server response: %s\n", server_reply);
        }
    };
    
    printf("Client> Test 1: Correct credentials (alice)\n");
    send_and_receive("LOGIN alice pass123");

    printf("\nClient> Test 2: Incorrect password (bob)\n");
    send_and_receive("LOGIN bob wrongpass");

    printf("\nClient> Test 3: User not found (dave)\n");
    send_and_receive("LOGIN dave somepass");

    printf("\nClient> Test 4: Correct credentials (charlie)\n");
    send_and_receive("LOGIN charlie qwerty");
    
    printf("\nClient> Test 5: Malformed request\n");
    send_and_receive("LOGI charlie qwerty");

    // Signal server shutdown
    char shutdown_msg[] = "SHUTDOWN\n";
    send(sock, shutdown_msg, strlen(shutdown_msg), 0);

    close_socket(sock);
    #ifdef _WIN32
    WSACleanup();
    #endif
    printf("--- Client Test Cases Finished ---\n");
}


int main() {
    // 1. Create user file
    FILE* file = fopen(USER_FILE, "w");
    if (file) {
        fprintf(file, "alice:pass123\n");
        fprintf(file, "bob:secret\n");
        fprintf(file, "charlie:qwerty\n");
        fclose(file);
    }
    
    // 2. Start server in a background thread
    THREAD_T server_thread_id;
    #ifdef _WIN32
    server_thread_id = CreateThread(NULL, 0, server_main, NULL, 0, NULL);
    Sleep(1000); // Wait 1 sec
    #else
    pthread_create(&server_thread_id, NULL, server_main, NULL);
    sleep(1); // Wait 1 sec
    #endif

    // 4. Run client test cases
    run_client();

    // 5. Wait for server thread to complete
    #ifdef _WIN32
    WaitForSingleObject(server_thread_id, INFINITE);
    CloseHandle(server_thread_id);
    #else
    pthread_join(server_thread_id, NULL);
    #endif

    // 6. Clean up
    remove(USER_FILE);
    printf("\nProgram finished.\n");
    return 0;
}