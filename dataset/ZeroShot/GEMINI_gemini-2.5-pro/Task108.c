/*
================================================================================
Instructions for C (Raw Sockets with OpenSSL):
1.  This code requires a C compiler (like gcc), and the OpenSSL development
    libraries (e.g., `libssl-dev` on Debian/Ubuntu).
2.  Generate a self-signed SSL certificate for the server. In your terminal, run:
    openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365 -nodes -subj "/CN=localhost"
3.  Compile the code. You must link against OpenSSL libraries.
    gcc -o task108 Task108.c -lssl -lcrypto -lpthread
4.  Start the server:
    ./task108 server
5.  In another terminal, run the client to execute test cases:
    ./task108 client
================================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <assert.h>

#define PORT 8443
#define CERT_FILE "cert.pem"
#define KEY_FILE "key.pem"
#define SECRET_TOKEN "secret-c-token-123"
#define MAX_BUFFER_SIZE 1024
#define MAX_KV_PAIRS 100
#define MAX_KEY_LEN 256
#define MAX_VAL_LEN 512

// --- In-memory key-value store (simulating an object) ---
typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
} KeyValuePair;

KeyValuePair data_store[MAX_KV_PAIRS];
int store_count = 0;

void store_put(const char* key, const char* value) {
    for (int i = 0; i < store_count; i++) {
        if (strncmp(data_store[i].key, key, MAX_KEY_LEN) == 0) {
            strncpy(data_store[i].value, value, MAX_VAL_LEN - 1);
            data_store[i].value[MAX_VAL_LEN - 1] = '\0';
            return;
        }
    }
    if (store_count < MAX_KV_PAIRS) {
        strncpy(data_store[store_count].key, key, MAX_KEY_LEN - 1);
        data_store[store_count].key[MAX_KEY_LEN - 1] = '\0';
        strncpy(data_store[store_count].value, value, MAX_VAL_LEN - 1);
        data_store[store_count].value[MAX_VAL_LEN - 1] = '\0';
        store_count++;
    }
}

const char* store_get(const char* key) {
    for (int i = 0; i < store_count; i++) {
        if (strncmp(data_store[i].key, key, MAX_KEY_LEN) == 0) {
            return data_store[i].value;
        }
    }
    return NULL;
}

int store_delete(const char* key) {
    for (int i = 0; i < store_count; i++) {
        if (strncmp(data_store[i].key, key, MAX_KEY_LEN) == 0) {
            // Overwrite with the last element
            if (i != store_count - 1) {
                memcpy(&data_store[i], &data_store[store_count - 1], sizeof(KeyValuePair));
            }
            store_count--;
            return 1; // Found and deleted
        }
    }
    return 0; // Not found
}

// --- OpenSSL Utilities ---
SSL_CTX *create_context(int is_server) {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    if (is_server) {
        method = TLS_server_method();
    } else {
        method = TLS_client_method();
    }
    
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    if (SSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_private_key_file(ctx, KEY_FILE, SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

// --- Server Implementation ---
void handle_connection(SSL *ssl) {
    char buffer[MAX_BUFFER_SIZE];
    int authenticated = 0;

    while (1) {
        int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) {
            int err = SSL_get_error(ssl, bytes);
            if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL) {
                 printf("Client disconnected.\n");
            } else {
                ERR_print_errors_fp(stderr);
            }
            break;
        }
        buffer[bytes] = '\0';

        // Simple text-based protocol: CMD KEY VALUE
        char cmd[16], key[MAX_KEY_LEN], value[MAX_VAL_LEN];
        sscanf(buffer, "%15s", cmd);

        if (!authenticated) {
            if (strcmp(cmd, "AUTH") == 0) {
                char token[MAX_BUFFER_SIZE];
                sscanf(buffer, "%*s %s", token);
                if (strcmp(token, SECRET_TOKEN) == 0) {
                    authenticated = 1;
                    SSL_write(ssl, "OK\n", 3);
                } else {
                    SSL_write(ssl, "ERROR_AUTH\n", 11);
                    break; // Close connection on failed auth
                }
            } else {
                 SSL_write(ssl, "ERROR_UNAUTHORIZED\n", 19);
                 break;
            }
            continue;
        }

        if (strcmp(cmd, "PUT") == 0) {
            sscanf(buffer, "%*s %s %s", key, value);
            store_put(key, value);
            SSL_write(ssl, "OK\n", 3);
        } else if (strcmp(cmd, "GET") == 0) {
            sscanf(buffer, "%*s %s", key);
            const char* val = store_get(key);
            if (val) {
                snprintf(buffer, sizeof(buffer), "VALUE %s\n", val);
                SSL_write(ssl, buffer, strlen(buffer));
            } else {
                SSL_write(ssl, "NOT_FOUND\n", 10);
            }
        } else if (strcmp(cmd, "DEL") == 0) {
            sscanf(buffer, "%*s %s", key);
            if (store_delete(key)) {
                SSL_write(ssl, "OK\n", 3);
            } else {
                SSL_write(ssl, "NOT_FOUND\n", 10);
            }
        } else {
            SSL_write(ssl, "ERROR_UNKNOWN_CMD\n", 18);
        }
    }
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

void start_server() {
    int sock;
    struct sockaddr_in addr;
    SSL_CTX *ctx;

    ctx = create_context(1);
    configure_context(ctx);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sock, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Starting secure C server on port %d\n", PORT);

    while(1) {
        struct sockaddr_in adr;
        unsigned int len = sizeof(adr);
        int client = accept(sock, (struct sockaddr*)&adr, &len);
        if (client < 0) {
            perror("Unable to accept");
            continue;
        }
        printf("Client connected.\n");

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        } else {
            handle_connection(ssl);
        }
        close(client);
    }

    close(sock);
    SSL_CTX_free(ctx);
}

// --- Client Implementation ---
void run_client_tests() {
    int sock;
    struct sockaddr_in addr;
    SSL_CTX *ctx;
    SSL *ssl;
    char buffer[MAX_BUFFER_SIZE];

    printf("Running client tests...\n");

    ctx = create_context(0);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("Unable to connect");
        exit(EXIT_FAILURE);
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    printf("SSL/TLS connection established.\n");

    // Authenticate first
    snprintf(buffer, sizeof(buffer), "AUTH %s", SECRET_TOKEN);
    SSL_write(ssl, buffer, strlen(buffer));
    SSL_read(ssl, buffer, sizeof(buffer)-1);
    assert(strncmp(buffer, "OK", 2) == 0);
    printf("Authentication successful.\n");

    // Test Case 1 & 2: Put and Get
    printf("\n--- Test Case 1 & 2: Put and Get ---\n");
    SSL_write(ssl, "PUT user1 data123", 17);
    SSL_read(ssl, buffer, sizeof(buffer)-1);
    printf("PUT response: %s", buffer);
    assert(strncmp(buffer, "OK", 2) == 0);
    SSL_write(ssl, "GET user1", 9);
    SSL_read(ssl, buffer, sizeof(buffer)-1);
    printf("GET response: %s", buffer);
    assert(strncmp(buffer, "VALUE data123", 13) == 0);

    // Test Case 3 & 4: Update and Get
    printf("\n--- Test Case 3 & 4: Update and Get ---\n");
    SSL_write(ssl, "PUT user1 data456", 17);
    SSL_read(ssl, buffer, sizeof(buffer)-1);
    printf("PUT response: %s", buffer);
    assert(strncmp(buffer, "OK", 2) == 0);
    SSL_write(ssl, "GET user1", 9);
    SSL_read(ssl, buffer, sizeof(buffer)-1);
    printf("GET response: %s", buffer);
    assert(strncmp(buffer, "VALUE data456", 13) == 0);

    // Test Case 5: Delete and Get
    printf("\n--- Test Case 5: Delete and Get ---\n");
    SSL_write(ssl, "DEL user1", 9);
    SSL_read(ssl, buffer, sizeof(buffer)-1);
    printf("DEL response: %s", buffer);
    assert(strncmp(buffer, "OK", 2) == 0);
    SSL_write(ssl, "GET user1", 9);
    SSL_read(ssl, buffer, sizeof(buffer)-1);
    printf("GET response: %s", buffer);
    assert(strncmp(buffer, "NOT_FOUND", 9) == 0);
    
    printf("\n--- Test Case 6: Authentication Failure ---\n");
    // Reconnect for this test
    close(sock);
    SSL_free(ssl);
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    SSL_connect(ssl);
    
    snprintf(buffer, sizeof(buffer), "AUTH %s", "wrong-token");
    SSL_write(ssl, buffer, strlen(buffer));
    int bytes = SSL_read(ssl, buffer, sizeof(buffer)-1);
    buffer[bytes] = '\0';
    printf("Bad AUTH response: %s", buffer);
    assert(strncmp(buffer, "ERROR_AUTH", 10) == 0);

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
    printf("\nAll client tests completed.\n");
}


int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server|client>\n", argv[0]);
        exit(1);
    }
    
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    if (strcmp(argv[1], "server") == 0) {
        start_server();
    } else if (strcmp(argv[1], "client") == 0) {
        run_client_tests();
    } else {
        fprintf(stderr, "Invalid mode. Use 'server' or 'client'.\n");
        exit(1);
    }
    
    EVP_cleanup();

    return 0;
}