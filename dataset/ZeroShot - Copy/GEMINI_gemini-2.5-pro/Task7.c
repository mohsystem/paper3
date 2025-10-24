#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    #define THREAD_HANDLE HANDLE
    #define THREAD_FUNC_RETURN DWORD
    #define THREAD_FUNC_DECL WINAPI
#else
    #include <pthread.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
    #define THREAD_HANDLE pthread_t
    #define THREAD_FUNC_RETURN void*
    #define THREAD_FUNC_DECL
#endif

// --- SHA256 Implementation (vendored for single-file requirement) ---
// Based on a public domain implementation by Brad Conte
typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned long long QWORD;

#define SHA256_BLOCK_SIZE 32
#define SHA256_STATE_SIZE 8

typedef struct {
    BYTE data[64];
    WORD datalen;
    QWORD bitlen;
    WORD state[SHA256_STATE_SIZE];
} SHA256_CTX;

static const WORD k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

void sha256_transform(SHA256_CTX *ctx, const BYTE data[]) {
    WORD a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for (; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx) {
    ctx->datalen = 0; ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a; ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}

void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void sha256_final(SHA256_CTX *ctx, BYTE hash[]) {
    WORD i = ctx->datalen;
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen; ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16; ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32; ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48; ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);
    for (i = 0; i < 4; ++i) {
        hash[i] = (ctx->state[0] >> (24 - i * 8)) & 0xff;
        hash[i + 4] = (ctx->state[1] >> (24 - i * 8)) & 0xff;
        hash[i + 8] = (ctx->state[2] >> (24 - i * 8)) & 0xff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0xff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0xff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0xff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0xff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0xff;
    }
}
// --- End of SHA256 Implementation ---


// --- Constants and Globals ---
#define PORT 12345
#define HOST "127.0.0.1"
#define USER_FILE "users.dat"
#define MAX_USERS 100
#define MAX_CLIENTS 10
#define BUFFER_SIZE 2048

char user_records[MAX_USERS][256];
int user_count = 0;

struct ClientInfo {
    SOCKET socket;
    char username[50];
};
struct ClientInfo connected_clients[MAX_CLIENTS];
int client_count = 0;

#ifdef _WIN32
    CRITICAL_SECTION client_mutex;
#else
    pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

// --- Utility Functions ---
void bytes_to_hex(const BYTE* bytes, size_t len, char* hex_str) {
    for(size_t i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
}

void hex_to_bytes(const char* hex_str, BYTE* bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        sscanf(hex_str + 2 * i, "%2hhx", &bytes[i]);
    }
}

void sha256_string(const char* str, char* hex_output) {
    BYTE hash[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE*)str, strlen(str));
    sha256_final(&ctx, hash);
    bytes_to_hex(hash, SHA256_BLOCK_SIZE, hex_output);
}

// NOTE: This is a weak substitute for a proper KDF like PBKDF2 or bcrypt.
void hash_password(const char* password, const char* salt_hex, char* output_hash) {
    char salted_password[256];
    snprintf(salted_password, sizeof(salted_password), "%s%s", password, salt_hex);

    char current_hash[65];
    sha256_string(salted_password, current_hash);

    for (int i = 0; i < 10000; i++) { // Key stretching
        sha256_string(current_hash, current_hash);
    }
    strcpy(output_hash, current_hash);
}

void generate_salt(char* salt_hex, size_t len) {
    BYTE salt[len];
    srand(time(NULL));
    for(size_t i = 0; i < len; i++) {
        salt[i] = rand() % 256;
    }
    bytes_to_hex(salt, len, salt_hex);
}

void print_usage() {
    printf("--- Secure Chat System ---\n");
    printf("1. To add a user (run this first):\n");
    printf("   ./Task7 adduser <username> <password>\n");
    printf("\n2. To start the server:\n");
    printf("   ./Task7 server\n");
    printf("\n3. To start the client (in a new terminal):\n");
    printf("   ./Task7 client\n");
    printf("\n--- Test Cases to run on Client ---\n");
    printf("1. Login with correct credentials.\n");
    printf("2. Try to login with correct user, wrong password.\n");
    printf("3. Try to login with a non-existent user.\n");
    printf("4. Login correctly and send a message.\n");
    printf("5. Try to login with an empty username or password.\n");
}

void add_user(const char* username, const char* password) {
    if (strlen(username) == 0 || strlen(password) == 0) {
        fprintf(stderr, "Username and password cannot be empty.\n");
        return;
    }
    FILE* file = fopen(USER_FILE, "a");
    if (!file) {
        perror("Error opening user file");
        return;
    }
    char salt[33];
    char hashed_pw[65];
    generate_salt(salt, 16);
    hash_password(password, salt, hashed_pw);
    fprintf(file, "%s:%s:%s\n", username, salt, hashed_pw);
    fclose(file);
    printf("User '%s' added successfully.\n", username);
}

void load_users() {
    FILE* file = fopen(USER_FILE, "r");
    if (!file) {
        fprintf(stderr, "User file not found. Please add users first.\n");
        return;
    }
    while(user_count < MAX_USERS && fgets(user_records[user_count], sizeof(user_records[0]), file)) {
        user_records[user_count][strcspn(user_records[user_count], "\r\n")] = 0; // trim newline
        user_count++;
    }
    fclose(file);
    printf("Loaded %d users.\n", user_count);
}

void trim_newline(char* str) {
    char* pos = strchr(str, '\n');
    if (pos) *pos = '\0';
    pos = strchr(str, '\r');
    if (pos) *pos = '\0';
}

void broadcast(const char* message) {
    printf("Broadcasting: %s", message);
#ifdef _WIN32
    EnterCriticalSection(&client_mutex);
#else
    pthread_mutex_lock(&client_mutex);
#endif
    for (int i = 0; i < client_count; i++) {
        send(connected_clients[i].socket, message, strlen(message), 0);
    }
#ifdef _WIN32
    LeaveCriticalSection(&client_mutex);
#else
    pthread_mutex_unlock(&client_mutex);
#endif
}

// --- Server Implementation ---
THREAD_FUNC_RETURN THREAD_FUNC_DECL handle_client(void* client_socket_ptr) {
    SOCKET client_socket = *(SOCKET*)client_socket_ptr;
    free(client_socket_ptr);
    char buffer[BUFFER_SIZE];
    bool authenticated = false;
    char username[50] = {0};

    send(client_socket, "SUBMIT_credentials\n", 19, 0);
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        closesocket(client_socket);
        return 0;
    }
    buffer[bytes_received] = '\0';
    trim_newline(buffer);

    char command[10], provided_user[50], provided_pass[50];
    if (sscanf(buffer, "%s %s %s", command, provided_user, provided_pass) == 3 && strcmp(command, "LOGIN") == 0) {
        for(int i = 0; i < user_count; i++) {
            char temp_record[256];
            strcpy(temp_record, user_records[i]);
            char* u = strtok(temp_record, ":");
            if (strcmp(u, provided_user) == 0) {
                char* salt = strtok(NULL, ":");
                char* stored_hash = strtok(NULL, ":");
                char provided_hash[65];
                hash_password(provided_pass, salt, provided_hash);
                if (strcmp(provided_hash, stored_hash) == 0) {
                    authenticated = true;
                    strcpy(username, provided_user);
                    break;
                }
            }
        }
    }

    if (!authenticated) {
        send(client_socket, "LOGIN_FAIL Invalid username or password.\n", 39, 0);
        closesocket(client_socket);
        return 0;
    }
    
    send(client_socket, "LOGIN_SUCCESS\n", 14, 0);
    
#ifdef _WIN32
    EnterCriticalSection(&client_mutex);
#else
    pthread_mutex_lock(&client_mutex);
#endif
    if (client_count < MAX_CLIENTS) {
        connected_clients[client_count].socket = client_socket;
        strcpy(connected_clients[client_count].username, username);
        client_count++;
    } else {
        closesocket(client_socket);
        #ifdef _WIN32
            LeaveCriticalSection(&client_mutex);
        #else
            pthread_mutex_unlock(&client_mutex);
        #endif
        return 0;
    }
#ifdef _WIN32
    LeaveCriticalSection(&client_mutex);
#else
    pthread_mutex_unlock(&client_mutex);
#endif
    
    char msg[100];
    snprintf(msg, sizeof(msg), "SERVER: %s has joined.\n", username);
    broadcast(msg);
    
    while((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        trim_newline(buffer);
        if (strcmp(buffer, "LOGOUT") == 0) {
            break;
        }
        char chat_msg[BUFFER_SIZE + 60];
        snprintf(chat_msg, sizeof(chat_msg), "%s: %s\n", username, buffer);
        broadcast(chat_msg);
    }
    
#ifdef _WIN32
    EnterCriticalSection(&client_mutex);
#else
    pthread_mutex_lock(&client_mutex);
#endif
    for(int i = 0; i < client_count; i++) {
        if (connected_clients[i].socket == client_socket) {
            for(int j = i; j < client_count - 1; j++) {
                connected_clients[j] = connected_clients[j+1];
            }
            client_count--;
            break;
        }
    }
#ifdef _WIN32
    LeaveCriticalSection(&client_mutex);
#else
    pthread_mutex_unlock(&client_mutex);
#endif

    snprintf(msg, sizeof(msg), "SERVER: %s has left.\n", username);
    broadcast(msg);
    closesocket(client_socket);
    return 0;
}

void run_server() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return;
    }
    InitializeCriticalSection(&client_mutex);
#endif

    load_users();
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        perror("Socket creation failed");
        return;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Bind failed");
        closesocket(server_socket);
        return;
    }
    
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        perror("Listen failed");
        closesocket(server_socket);
        return;
    }
    
    printf("Server started on port %d\n", PORT);
    
    while(1) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            perror("Accept failed");
            continue;
        }
        
        SOCKET* p_client_sock = malloc(sizeof(SOCKET));
        *p_client_sock = client_socket;

        THREAD_HANDLE thread_id;
#ifdef _WIN32
        thread_id = CreateThread(NULL, 0, handle_client, p_client_sock, 0, NULL);
        if (thread_id == NULL) {
             perror("CreateThread failed");
             free(p_client_sock);
        }
#else
        if (pthread_create(&thread_id, NULL, handle_client, p_client_sock) != 0) {
            perror("pthread_create failed");
            free(p_client_sock);
        }
        pthread_detach(thread_id);
#endif
    }
    
    closesocket(server_socket);
#ifdef _WIN32
    WSACleanup();
    DeleteCriticalSection(&client_mutex);
#endif
}


// --- Client Implementation ---
THREAD_FUNC_RETURN THREAD_FUNC_DECL receive_messages(void* sock_ptr) {
    SOCKET sock = *(SOCKET*)sock_ptr;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
        fflush(stdout);
    }
    if (bytes_received <= 0) {
        printf("\nConnection to server lost.\n");
        closesocket(sock);
        exit(0);
    }
    return 0;
}

void run_client() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n"); return;
    }
#endif
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        perror("Socket creation failed"); return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Connection failed");
        closesocket(client_socket);
        return;
    }
    printf("Connected to server.\n");

    char buffer[BUFFER_SIZE];
    int bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        if (strstr(buffer, "SUBMIT_credentials")) {
            char username[50], password[50];
            printf("Enter username: ");
            scanf("%s", username);
            printf("Enter password: ");
            scanf("%s", password);

            char login_msg[128];
            snprintf(login_msg, sizeof(login_msg), "LOGIN %s %s\n", username, password);
            send(client_socket, login_msg, strlen(login_msg), 0);
            
            bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if(bytes > 0) {
                buffer[bytes] = '\0';
                printf("Server: %s", buffer);
                if (strstr(buffer, "LOGIN_SUCCESS")) {
                    printf("Login successful! You can now chat. Type 'LOGOUT' to exit.\n");
                    
                    THREAD_HANDLE recv_thread;
#ifdef _WIN32
                    recv_thread = CreateThread(NULL, 0, receive_messages, &client_socket, 0, NULL);
#else
                    pthread_create(&recv_thread, NULL, receive_messages, &client_socket);
#endif
                    char line[BUFFER_SIZE];
                    // consume newline from previous scanf
                    while (getchar() != '\n'); 
                    while (fgets(line, sizeof(line), stdin)) {
                        trim_newline(line);
                        if(strcmp(line, "LOGOUT") == 0) {
                           send(client_socket, "LOGOUT\n", 7, 0);
                           break; 
                        }
                        send(client_socket, line, strlen(line), 0);
                    }
#ifdef _WIN32
                    TerminateThread(recv_thread, 0);
                    CloseHandle(recv_thread);
#else
                    pthread_cancel(recv_thread);
#endif
                }
            }
        }
    }
    closesocket(client_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}


// --- Main Dispatcher ---
int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    if (strcmp(argv[1], "server") == 0) {
        run_server();
    } else if (strcmp(argv[1], "client") == 0) {
        run_client();
    } else if (strcmp(argv[1], "adduser") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: ./Task7 adduser <username> <password>\n");
            return 1;
        }
        add_user(argv[2], argv[3]);
    } else {
        print_usage();
        return 1;
    }

    return 0;
}