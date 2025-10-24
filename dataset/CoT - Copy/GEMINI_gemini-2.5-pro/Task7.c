#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

// Socket-related headers for POSIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// --- !! SECURITY WARNING !! ---
// This code sends passwords in PLAINTEXT over the network.
// It is NOT secure and should ONLY be used for educational purposes.
// A production system MUST use TLS/SSL to encrypt all communication.
// -----------------------------

#define PORT 8083
#define BUFFER_SIZE 1024
#define SALT_LEN 16
#define HASH_LEN 32
#define MAX_LINE_LEN 256
#define USER_FILE "users.dat"

// --- SHA256 Implementation (Compact, self-contained) ---
// Based on public domain work.
typedef struct {
    uint8_t buffer[64];
    uint32_t state[8];
    uint64_t bitcount;
} sha256_context;

static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define BSIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SSIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SSIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

void sha256_transform(sha256_context *ctx) {
    uint32_t a = ctx->state[0], b = ctx->state[1], c = ctx->state[2], d = ctx->state[3], e = ctx->state[4], f = ctx->state[5], g = ctx->state[6], h = ctx->state[7];
    uint32_t w[64];
    for (int i = 0; i < 16; ++i)
        w[i] = (ctx->buffer[4*i] << 24) | (ctx->buffer[4*i+1] << 16) | (ctx->buffer[4*i+2] << 8) | ctx->buffer[4*i+3];
    for (int i = 16; i < 64; ++i)
        w[i] = SSIG1(w[i - 2]) + w[i - 7] + SSIG0(w[i - 15]) + w[i - 16];
    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + BSIG1(e) + CH(e, f, g) + K[i] + w[i];
        uint32_t t2 = BSIG0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void sha256_init(sha256_context *ctx) {
    ctx->bitcount = 0;
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c; ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
}
void sha256_update(sha256_context *ctx, const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->buffer[ctx->bitcount % 64] = data[i];
        if (++ctx->bitcount % 64 == 0) sha256_transform(ctx);
    }
}
void sha256_final(sha256_context *ctx, uint8_t *hash) {
    uint8_t pad[64];
    uint64_t i = ctx->bitcount % 64;
    pad[0] = 0x80;
    memset(pad + 1, 0, 64 - (i + 1));
    if (i >= 56) {
        sha256_update(ctx, pad, 64 - i);
        memset(pad, 0, 56);
    } else {
        sha256_update(ctx, pad, 56 - i);
    }
    uint64_t bits = ctx->bitcount * 8;
    for (i = 0; i < 8; ++i) pad[56+i] = (bits >> (56-8*i)) & 0xFF;
    sha256_update(ctx, pad, 8);
    for (i = 0; i < 8; i++) {
        hash[i*4]   = (ctx->state[i] >> 24) & 0xFF;
        hash[i*4+1] = (ctx->state[i] >> 16) & 0xFF;
        hash[i*4+2] = (ctx->state[i] >> 8) & 0xFF;
        hash[i*4+3] = ctx->state[i] & 0xFF;
    }
}

// --- Security and Helper Utilities ---
void bytes_to_hex(const uint8_t* bytes, size_t len, char* hex_str) {
    for(size_t i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
}
void hex_to_bytes(const char* hex_str, uint8_t* bytes, size_t len) {
    for(size_t i = 0; i < len; i++) {
        sscanf(hex_str + 2 * i, "%2hhx", &bytes[i]);
    }
}

void hash_password(const char* password, const uint8_t* salt, uint8_t* hash_out) {
    sha256_context ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, salt, SALT_LEN);
    sha256_update(&ctx, (const uint8_t*)password, strlen(password));
    sha256_final(&ctx, hash_out);
}

void generate_salt(uint8_t* salt) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        fread(salt, 1, SALT_LEN, f);
        fclose(f);
    }
}

void add_user(const char* username, const char* password) {
    FILE* file = fopen(USER_FILE, "r");
    if(file) {
        char line[MAX_LINE_LEN];
        while(fgets(line, sizeof(line), file)) {
            char* u = strtok(line, ":");
            if (u && strcmp(u, username) == 0) {
                printf("Error: User '%s' already exists.\n", username);
                fclose(file);
                return;
            }
        }
        fclose(file);
    }
    
    file = fopen(USER_FILE, "a");
    if(!file) {
        perror("Error opening user file");
        return;
    }

    uint8_t salt[SALT_LEN];
    uint8_t hash[HASH_LEN];
    char salt_hex[SALT_LEN * 2 + 1];
    char hash_hex[HASH_LEN * 2 + 1];

    generate_salt(salt);
    hash_password(password, salt, hash);
    
    bytes_to_hex(salt, SALT_LEN, salt_hex);
    bytes_to_hex(hash, HASH_LEN, hash_hex);
    
    fprintf(file, "%s:%s:%s\n", username, hash_hex, salt_hex);
    fclose(file);
    printf("User '%s' added successfully.\n", username);
}

int authenticate(const char* username, const char* password) {
    FILE* file = fopen(USER_FILE, "r");
    if (!file) return 0;
    
    char line[MAX_LINE_LEN];
    while(fgets(line, sizeof(line), file)) {
        char *u, *h, *s;
        u = strtok(line, ":");
        h = strtok(NULL, ":");
        s = strtok(NULL, "\n");

        if (u && h && s && strcmp(u, username) == 0) {
            uint8_t stored_hash[HASH_LEN];
            uint8_t salt[SALT_LEN];
            uint8_t computed_hash[HASH_LEN];

            hex_to_bytes(h, stored_hash, HASH_LEN);
            hex_to_bytes(s, salt, SALT_LEN);
            
            hash_password(password, salt, computed_hash);

            fclose(file);
            return memcmp(stored_hash, computed_hash, HASH_LEN) == 0;
        }
    }
    fclose(file);
    return 0;
}

// --- Server ---
void *handle_client(void *socket_desc) {
    int client_sock = *(int*)socket_desc;
    free(socket_desc);
    char buffer[BUFFER_SIZE] = {0};

    if (recv(client_sock, buffer, BUFFER_SIZE - 1, 0) <= 0) {
        close(client_sock);
        return NULL;
    }
    
    char* command = strtok(buffer, " ");
    char* username = strtok(NULL, " ");
    char* password = strtok(NULL, "\r\n");

    if (command && username && password) {
        printf("Server received: %s %s *****\n", command, username);
        if (strcmp(command, "LOGIN") == 0 && authenticate(username, password)) {
            send(client_sock, "LOGIN_SUCCESS", 13, 0);
             printf("Authentication successful for %s\n", username);
        } else {
            send(client_sock, "LOGIN_FAIL", 10, 0);
            printf("Authentication failed for %s\n", username);
        }
    } else {
        send(client_sock, "INVALID_REQUEST", 15, 0);
    }

    close(client_sock);
    return NULL;
}

void run_server() {
    int server_fd, *new_sock;
    struct sockaddr_in server;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Could not create socket");
        return;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed");
        close(server_fd);
        return;
    }

    listen(server_fd, 3);
    printf("Server listening on port %d\n", PORT);

    while(1) {
        int client_sock = accept(server_fd, NULL, NULL);
        if (client_sock < 0) {
            perror("accept failed");
            continue;
        }

        pthread_t sniffer_thread;
        new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, handle_client, (void*) new_sock) < 0) {
            perror("could not create thread");
            free(new_sock);
        }
    }
    close(server_fd);
}

// --- Client ---
void run_client(const char* username, const char* password) {
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed. Is the server running?\n");
        close(sock);
        return;
    }

    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "LOGIN %s %s", username, password);
    send(sock, request, strlen(request), 0);
    printf("Client sent login request for user: %s\n", username);

    recv(sock, buffer, BUFFER_SIZE - 1, 0);
    printf("Server response: %s\n", buffer);
    close(sock);
}


void print_usage() {
    printf("Usage: ./Task7 <mode> [options]\n");
    printf("Modes:\n");
    printf("  server                  - Start the chat server.\n");
    printf("  adduser <user> <pass>   - Add a new user to the user file.\n");
    printf("  client                  - Run predefined client test cases.\n");
    printf("Compile with: gcc -o Task7 your_file_name.c -pthread\n");
}

void run_client_tests() {
    printf("\n--- Running Client Test Cases ---\n");
    printf("Instructions:\n");
    printf("1. Compile the code: gcc -o Task7 your_file_name.c -pthread\n");
    printf("2. Start the server in a separate terminal: ./Task7 server\n");
    printf("3. Add a test user in another terminal: ./Task7 adduser testuser testpass123\n");
    printf("4. The test cases below will now run against the server.\n\n");

    printf("--- Test Case 1: Successful Login ---\n");
    run_client("testuser", "testpass123");
    
    printf("\n--- Test Case 2: Incorrect Password ---\n");
    run_client("testuser", "wrongpassword");

    printf("\n--- Test Case 3: Non-existent User ---\n");
    run_client("nouser", "anypassword");

    printf("\n--- Test Case 4: Another Successful Login ---\n");
    run_client("testuser", "testpass123");

    printf("\n--- Test Case 5: Empty Password ---\n");
    run_client("testuser", "");
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        run_server();
    } else if (strcmp(argv[1], "adduser") == 0) {
        if (argc != 4) {
            printf("Usage: ./Task7 adduser <username> <password>\n");
            return 1;
        }
        add_user(argv[2], argv[3]);
    } else if (strcmp(argv[1], "client") == 0) {
        run_client_tests();
    } else {
        print_usage();
    }
    
    return 0;
}