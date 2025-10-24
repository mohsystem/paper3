
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    #define sleep(x) Sleep((x) * 1000)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define closesocket close
#endif

#define MAX_USERS 100
#define BUFFER_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
} User;

typedef struct {
    User users[MAX_USERS];
    int user_count;
} UserDatabase;

UserDatabase db;

void load_users() {
    FILE* file = fopen("users.txt", "r");
    if (!file) {
        file = fopen("users.txt", "w");
        fprintf(file, "alice:password123\\n");
        fprintf(file, "bob:securepass\\n");
        fprintf(file, "charlie:mypass456\\n");
        fprintf(file, "david:test1234\\n");
        fprintf(file, "eve:qwerty789\\n");
        fclose(file);
        file = fopen("users.txt", "r");
    }
    
    db.user_count = 0;
    char line[100];
    while (fgets(line, sizeof(line), file) && db.user_count < MAX_USERS) {
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\\0';
            strcpy(db.users[db.user_count].username, line);
            strcpy(db.users[db.user_count].password, colon + 1);
            db.users[db.user_count].password[strcspn(db.users[db.user_count].password, "\\n")] = 0;
            db.user_count++;
        }
    }
    fclose(file);
}

void process_request(const char* request, char* response) {
    if (strncmp(request, "LOGIN:", 6) != 0) {
        strcpy(response, "ERROR:Invalid request format");
        return;
    }
    
    char username[50], password[50];
    char* first_colon = strchr(request, ':');
    char* second_colon = strchr(first_colon + 1, ':');
    
    if (!second_colon) {
        strcpy(response, "ERROR:Invalid login format");
        return;
    }
    
    int username_len = second_colon - first_colon - 1;
    strncpy(username, first_colon + 1, username_len);
    username[username_len] = '\\0';
    strcpy(password, second_colon + 1);
    
    for (int i = 0; i < db.user_count; i++) {
        if (strcmp(db.users[i].username, username) == 0 && 
            strcmp(db.users[i].password, password) == 0) {
            strcpy(response, "SUCCESS:Login successful");
            return;
        }
    }
    
    strcpy(response, "ERROR:Invalid username or password");
}

#ifdef _WIN32
DWORD WINAPI handle_client(LPVOID arg) {
#else
void* handle_client(void* arg) {
#endif
    SOCKET client_socket = *(SOCKET*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE];
    
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    process_request(buffer, response);
    send(client_socket, response, strlen(response), 0);
    
    closesocket(client_socket);
    return 0;
}

#ifdef _WIN32
DWORD WINAPI server_thread(LPVOID arg) {
#else
void* server_thread(void* arg) {
#endif
    SOCKET server_socket;
    struct sockaddr_in server_addr, client_addr;
    
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);
    
    printf("Server started on port 8080\\n");
    
    while (1) {
        int client_len = sizeof(client_addr);
        SOCKET* client_socket = malloc(sizeof(SOCKET));
        *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        #ifdef _WIN32
        CreateThread(NULL, 0, handle_client, client_socket, 0, NULL);
        #else
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_socket);
        pthread_detach(tid);
        #endif
    }
    
    return 0;
}

void chat_client_login(const char* username, const char* password, char* response) {
    SOCKET sock;
    struct sockaddr_in server_addr;
    
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "LOGIN:%s:%s", username, password);
    send(sock, request, strlen(request), 0);
    
    recv(sock, response, BUFFER_SIZE, 0);
    
    closesocket(sock);
    #ifdef _WIN32
    WSACleanup();
    #endif
}

int main() {
    printf("=== Chat System Test Cases ===\\n\\n");
    
    load_users();
    
    #ifdef _WIN32
    CreateThread(NULL, 0, server_thread, NULL, 0, NULL);
    #else
    pthread_t tid;
    pthread_create(&tid, NULL, server_thread, NULL);
    pthread_detach(tid);
    #endif
    
    sleep(1);
    
    const char* test_cases[][2] = {
        {"alice", "password123"},
        {"bob", "securepass"},
        {"charlie", "wrongpass"},
        {"david", "test1234"},
        {"unknown", "password"}
    };
    
    for (int i = 0; i < 5; i++) {
        char response[BUFFER_SIZE] = {0};
        printf("Test Case %d:\\n", i + 1);
        printf("Username: %s\\n", test_cases[i][0]);
        printf("Password: %s\\n", test_cases[i][1]);
        
        chat_client_login(test_cases[i][0], test_cases[i][1], response);
        printf("Response: %s\\n\\n", response);
        
        #ifdef _WIN32
        Sleep(500);
        #else
        usleep(500000);
        #endif
    }
    
    return 0;
}
