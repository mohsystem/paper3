#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

// POSIX/Linux headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/file.h>
#include <termios.h>

// OpenSSL headers
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

// --- Constants ---
#define PORT 8080
#define BUFFER_SIZE 1024
#define CREDENTIALS_FILE "users.dat"
#define MIN_PASSWORD_LEN 8
#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 64
#define LINE_BUFFER_SIZE 256

// --- Hashing Constants ---
#define SALT_SIZE 16
#define HASH_SIZE 32 // SHA-256
#define PBKDF2_ITERATIONS 210000

// --- Utility Functions ---

// Securely clear memory
void secure_zero_memory(void* v, size_t n) {
#if defined(_MSC_VER)
    SecureZeroMemory(v, n);
#else
    volatile unsigned char* p = v;
    while (n--) *p++ = 0;
#endif
}

void bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_str) {
    for (size_t i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

int hex_to_bytes(const char* hex_str, unsigned char* bytes, size_t max_len) {
    size_t hex_len = strlen(hex_str);
    if (hex_len % 2 != 0 || (hex_len / 2) > max_len) {
        return -1;
    }
    for (size_t i = 0; i < hex_len / 2; i++) {
        sscanf(hex_str + 2 * i, "%2hhx", &bytes[i]);
    }
    return hex_len / 2;
}

// --- Cryptography and Authentication ---

int hash_password(const char* password, unsigned char* salt, char* hash_hex) {
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        fprintf(stderr, "Failed to generate salt.\n");
        return -1;
    }

    unsigned char hash[HASH_SIZE];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt, SALT_SIZE,
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          HASH_SIZE, hash) != 1) {
        fprintf(stderr, "Failed to hash password.\n");
        return -1;
    }
    
    bytes_to_hex(hash, HASH_SIZE, hash_hex);
    secure_zero_memory(hash, HASH_SIZE);
    return 0;
}

bool verify_password(const char* password, const char* stored_record) {
    char username_part[MAX_USERNAME_LEN + 1];
    char salt_hex[SALT_SIZE * 2 + 1];
    char hash_hex[HASH_SIZE * 2 + 1];
    
    if (sscanf(stored_record, "%[^:]:%[^:]:%s", username_part, salt_hex, hash_hex) != 3) {
        return false;
    }

    unsigned char salt[SALT_SIZE];
    unsigned char stored_hash[HASH_SIZE];
    
    if (hex_to_bytes(salt_hex, salt, SALT_SIZE) != SALT_SIZE) return false;
    if (hex_to_bytes(hash_hex, stored_hash, HASH_SIZE) != HASH_SIZE) return false;

    unsigned char calculated_hash[HASH_SIZE];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt, SALT_SIZE,
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          HASH_SIZE, calculated_hash) != 1) {
        secure_zero_memory(calculated_hash, HASH_SIZE);
        return false;
    }
    
    bool result = CRYPTO_memcmp(calculated_hash, stored_hash, HASH_SIZE) == 0;
    secure_zero_memory(calculated_hash, HASH_SIZE);
    return result;
}


// --- User Management ---

bool is_valid_username(const char* username) {
    size_t len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LEN) return false;
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return false;
        }
    }
    return true;
}

bool user_exists(const char* username) {
    FILE* file = fopen(CREDENTIALS_FILE, "r");
    if (!file) return false;

    char line[LINE_BUFFER_SIZE];
    char record_user[MAX_USERNAME_LEN + 1];
    bool found = false;

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%[^:]:", record_user) == 1) {
            if (strncmp(username, record_user, MAX_USERNAME_LEN) == 0) {
                found = true;
                break;
            }
        }
    }
    fclose(file);
    return found;
}

int add_user(const char* username, char* password) {
    if (!is_valid_username(username)) {
        fprintf(stderr, "Invalid username format or length.\n");
        return -1;
    }
    size_t pass_len = strlen(password);
    if (pass_len < MIN_PASSWORD_LEN || pass_len > MAX_PASSWORD_LEN) {
        fprintf(stderr, "Password does not meet length requirements.\n");
        return -1;
    }

    int fd = open(CREDENTIALS_FILE, O_CREAT | O_WRONLY | O_APPEND, 0600);
    if (fd == -1) {
        perror("Could not open credentials file for writing");
        return -1;
    }
    
    if (flock(fd, LOCK_EX) != 0) {
        perror("Could not lock credentials file");
        close(fd);
        return -1;
    }

    if (user_exists(username)) {
        fprintf(stderr, "User already exists.\n");
        flock(fd, LOCK_UN);
        close(fd);
        return -1;
    }

    unsigned char salt[SALT_SIZE];
    char hash_hex[HASH_SIZE * 2 + 1];
    if (hash_password(password, salt, hash_hex) != 0) {
        flock(fd, LOCK_UN);
        close(fd);
        return -1;
    }
    secure_zero_memory(password, strlen(password));
    
    char salt_hex[SALT_SIZE * 2 + 1];
    bytes_to_hex(salt, SALT_SIZE, salt_hex);

    char record[LINE_BUFFER_SIZE];
    int len = snprintf(record, sizeof(record), "%s:%s:%s\n", username, salt_hex, hash_hex);
    if (len < 0 || (size_t)len >= sizeof(record)) {
         fprintf(stderr, "Error formatting record.\n");
         flock(fd, LOCK_UN);
         close(fd);
         return -1;
    }

    if (write(fd, record, len) != len) {
        perror("Failed to write to credentials file");
        flock(fd, LOCK_UN);
        close(fd);
        return -1;
    }

    flock(fd, LOCK_UN);
    close(fd);
    printf("User '%s' added successfully.\n", username);
    return 0;
}

bool authenticate(const char* username, char* password) {
    if (!is_valid_username(username)) return false;

    FILE* file = fopen(CREDENTIALS_FILE, "r");
    if (!file) {
        perror("Could not open credentials file for reading");
        return false;
    }

    char line[LINE_BUFFER_SIZE];
    char record_user[MAX_USERNAME_LEN + 1];
    bool authenticated = false;

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%[^:]:", record_user) == 1) {
            if (strncmp(username, record_user, MAX_USERNAME_LEN) == 0) {
                authenticated = verify_password(password, line);
                break;
            }
        }
    }
    
    fclose(file);
    secure_zero_memory(password, strlen(password));
    return authenticated;
}


// --- Server Logic ---
void* handle_client(void* client_socket_ptr) {
    int client_socket = *(int*)client_socket_ptr;
    free(client_socket_ptr);

    char buffer[BUFFER_SIZE];
    bool is_authenticated = false;

    while (true) {
        ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            printf("Client disconnected or error.\n");
            break;
        }
        buffer[bytes_read] = '\0';
        
        // Trim trailing newline characters
        char* pos = strchr(buffer, '\n'); if (pos) *pos = '\0';
        pos = strchr(buffer, '\r'); if (pos) *pos = '\0';
        
        char command[10] = {0}, username[MAX_USERNAME_LEN + 1] = {0};
        char password[MAX_PASSWORD_LEN + 1] = {0};
        
        sscanf(buffer, "%9s", command);

        if (!is_authenticated) {
            if (strcmp(command, "LOGIN") == 0) {
                 if (sscanf(buffer, "%*s %32s %64s", username, password) == 2) {
                     if (authenticate(username, password)) {
                        is_authenticated = true;
                        send(client_socket, "OK\n", 3, 0);
                     } else {
                        send(client_socket, "FAIL\n", 5, 0);
                     }
                 } else {
                    send(client_socket, "FAIL Invalid LOGIN format\n", 26, 0);
                 }
            } else {
                 send(client_socket, "FAIL Please LOGIN first\n", 25, 0);
            }
        } else {
            if (strcmp(command, "MSG") == 0) {
                char* msg = buffer + 4; // Skip "MSG "
                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response), "ECHO: %s\n", msg);
                send(client_socket, response, strlen(response), 0);
            } else if (strcmp(command, "LOGOUT") == 0) {
                break;
            } else {
                send(client_socket, "FAIL Unknown command\n", 21, 0);
            }
        }
    }
    close(client_socket);
    return NULL;
}

void run_server() {
    // In a real application, you should use TLS/SSL to encrypt communication.
    // This example uses plain TCP sockets for simplicity.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }
    
    printf("Server listening on port %d\n", PORT);

    while (true) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        printf("New client connected\n");
        
        pthread_t tid;
        int* client_sock_ptr = malloc(sizeof(int));
        if (!client_sock_ptr) {
            perror("malloc for client socket");
            close(client_socket);
            continue;
        }
        *client_sock_ptr = client_socket;
        
        if (pthread_create(&tid, NULL, handle_client, client_sock_ptr) != 0) {
            perror("pthread_create");
            free(client_sock_ptr);
            close(client_socket);
        }
        pthread_detach(tid);
    }
    close(server_fd);
}


// --- Client Logic ---
void get_password_no_echo(char* password, size_t max_len) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    if (fgets(password, max_len, stdin) != NULL) {
        // Remove newline character
        password[strcspn(password, "\n")] = 0;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}


void run_client(const char* server_ip) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address/ Address not supported\n");
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock);
        return;
    }

    char username[MAX_USERNAME_LEN + 1];
    char password[MAX_PASSWORD_LEN + 1];
    char buffer[BUFFER_SIZE];

    printf("Enter username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) return;
    username[strcspn(username, "\n")] = 0;

    printf("Enter password: ");
    get_password_no_echo(password, sizeof(password));

    snprintf(buffer, sizeof(buffer), "LOGIN %s %s\n", username, password);
    send(sock, buffer, strlen(buffer), 0);
    secure_zero_memory(password, sizeof(password));

    ssize_t bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        if (strncmp(buffer, "OK\n", 3) == 0) {
            printf("Login successful. Type 'MSG <your_message>' or 'LOGOUT'.\n");
            while (true) {
                printf("> ");
                if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
                
                if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] != '\n') {
                    // Input too long, clear stdin
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                    printf("Message too long.\n");
                    continue;
                }

                send(sock, buffer, strlen(buffer), 0);

                if (strncmp(buffer, "LOGOUT", 6) == 0) break;

                bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    printf("Server: %s", buffer);
                } else {
                    printf("Server disconnected.\n");
                    break;
                }
            }
        } else {
            fprintf(stderr, "Login failed. Server response: %s", buffer);
        }
    } else {
        fprintf(stderr, "No response from server or connection closed.\n");
    }
    close(sock);
}


// --- Main ---
void print_usage(const char* prog_name) {
    fprintf(stderr, "Usage:\n"
              "  %s server\n"
              "  %s client <server_ip>\n"
              "  %s adduser <username> <password>\n", prog_name, prog_name, prog_name);
}


int main(int argc, char* argv[]) {
    // ---- Test Cases ----
    // To test the system, run the following commands in separate terminals.
    //
    // Test Case 1: Add a valid user.
    //   ./program adduser testuser StrongPwd123!
    //   Expected: User 'testuser' added successfully.
    //
    // Test Case 2: Start the server.
    //   ./program server
    //   Expected: Server listens on port 8080.
    //
    // Test Case 3: Client connects with correct credentials and chats.
    //   ./program client 127.0.0.1
    //   (At prompt, enter 'testuser' and 'StrongPwd123!')
    //   Expected: Login successful. Can send and receive messages.
    //
    // Test Case 4: Client connects with incorrect credentials.
    //   ./program client 127.0.0.1
    //   (At prompt, enter 'testuser' and 'wrongpassword')
    //   Expected: Login failed.
    //
    // Test Case 5: Attempt to add a user with an invalid username.
    //   ./program adduser ../../etc/passwd bad
    //   Expected: Error message about invalid username format.
    // --------------------

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        run_server();
    } else if (strcmp(argv[1], "client") == 0) {
        if (argc != 3) {
            print_usage(argv[0]);
            return 1;
        }
        run_client(argv[2]);
    } else if (strcmp(argv[1], "adduser") == 0) {
        if (argc != 4) {
            print_usage(argv[0]);
            return 1;
        }
        char* password = argv[3];
        if (add_user(argv[2], password) != 0) {
            return 1;
        }
    } else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}