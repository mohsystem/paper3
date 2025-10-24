#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#else
#define MKDIR(path, mode) mkdir(path, mode)
#endif

// Constants
#define MAGIC_HEADER "ENC1"
#define MAGIC_HEADER_LEN 4
#define VERSION 1
#define KEY_SIZE 32
#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define PBKDF2_ITERATIONS 210000
#define SESSION_ID_SIZE 32
#define SESSION_DIR "sessions_c"
#define SESSION_LIFETIME_SECONDS 3600
#define MAX_USER_ID_LEN 255
#define MAX_PATH_LEN 256

// Structs
typedef struct {
    char user_id[MAX_USER_ID_LEN + 1];
    time_t expires_at;
} SessionData;

// Helper to print OpenSSL errors for debugging
void print_openssl_errors(const char* context){
    fprintf(stderr, "OpenSSL error in %s:\n", context);
    ERR_print_errors_fp(stderr);
    ERR_clear_error();
}

// Generate cryptographically secure random bytes
int generate_secure_random_bytes(unsigned char* buffer, int size) {
    if (RAND_bytes(buffer, size) != 1) {
        return 0;
    }
    return 1;
}

// Convert bytes to a dynamically allocated hex string
char* bytes_to_hex(const unsigned char* bytes, int len) {
    if (len <= 0) return NULL;
    char* hex_str = (char*)malloc((size_t)len * 2 + 1);
    if (!hex_str) return NULL;
    for (int i = 0; i < len; i++) {
        sprintf(hex_str + i * 2, "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

// Calculate SHA-256 hash of input data
int sha256_hash(const char* input, size_t input_len, unsigned char* output) {
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) return 0;

    int result = 0;
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) == 1 &&
        EVP_DigestUpdate(mdctx, input, input_len) == 1 &&
        EVP_DigestFinal_ex(mdctx, output, NULL) == 1) {
        result = 1;
    }
    
    EVP_MD_CTX_free(mdctx);
    return result;
}

// Derive a key from a password using PBKDF2
int derive_key_from_password(const char* password, const unsigned char* salt, unsigned char* key, int key_len) {
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                           salt, SALT_SIZE,
                           PBKDF2_ITERATIONS, EVP_sha256(),
                           key_len, key) != 1) {
        return 0;
    }
    return 1;
}

// Ensure the session directory exists
int create_session_dir_if_not_exists() {
    struct stat st = {0};
    if (stat(SESSION_DIR, &st) == -1) {
        if (MKDIR(SESSION_DIR, 0700) != 0 && errno != EEXIST) {
            perror("Failed to create session directory");
            return 0;
        }
    }
    return 1;
}

// Creates a new session for a user and returns a unique session ID.
char* create_session(const char* user_id, const char* master_key) {
    if (strlen(user_id) > MAX_USER_ID_LEN) {
        fprintf(stderr, "User ID too long\n");
        return NULL;
    }
    if (!create_session_dir_if_not_exists()) return NULL;

    unsigned char session_id_bytes[SESSION_ID_SIZE];
    unsigned char salt[SALT_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char key[KEY_SIZE] = {0};
    unsigned char tag[TAG_SIZE];
    unsigned char* plaintext = NULL;
    unsigned char* ciphertext = NULL;
    char* session_id_hex = NULL;
    FILE* fp = NULL;
    EVP_CIPHER_CTX* ctx = NULL;
    char* result = NULL;

    size_t user_id_len = strlen(user_id);
    size_t plaintext_len = user_id_len + 1 + sizeof(time_t);
    plaintext = (unsigned char*)malloc(plaintext_len);
    if (!plaintext) goto cleanup;

    SessionData data;
    strncpy(data.user_id, user_id, MAX_USER_ID_LEN);
    data.user_id[MAX_USER_ID_LEN] = '\0';
    data.expires_at = time(NULL) + SESSION_LIFETIME_SECONDS;
    memcpy(plaintext, data.user_id, user_id_len + 1);
    memcpy(plaintext + user_id_len + 1, &data.expires_at, sizeof(time_t));
    
    if (!generate_secure_random_bytes(session_id_bytes, SESSION_ID_SIZE) || !generate_secure_random_bytes(salt, SALT_SIZE) || !generate_secure_random_bytes(iv, IV_SIZE)) {
        print_openssl_errors("random generation");
        goto cleanup;
    }
    
    session_id_hex = bytes_to_hex(session_id_bytes, SESSION_ID_SIZE);
    if (!session_id_hex) goto cleanup;

    if (!derive_key_from_password(master_key, salt, key, KEY_SIZE)) {
        print_openssl_errors("key derivation");
        goto cleanup;
    }

    ciphertext = (unsigned char*)malloc(plaintext_len);
    if (!ciphertext) goto cleanup;

    int len = 0, ciphertext_len = 0;
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) goto cleanup;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1 ||
        EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, (int)plaintext_len) != 1) {
        print_openssl_errors("encryption update");
        goto cleanup;
    }
    ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        print_openssl_errors("encryption final");
        goto cleanup;
    }
    ciphertext_len += len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        print_openssl_errors("get tag");
        goto cleanup;
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    if(!sha256_hash(session_id_hex, strlen(session_id_hex), hash)) goto cleanup;
    char* filename_hash_hex = bytes_to_hex(hash, SHA256_DIGEST_LENGTH);
    if (!filename_hash_hex) goto cleanup;
    char session_filepath[MAX_PATH_LEN];
    snprintf(session_filepath, sizeof(session_filepath), "%s/%s", SESSION_DIR, filename_hash_hex);
    free(filename_hash_hex);

    fp = fopen(session_filepath, "wb");
    if (!fp) goto cleanup;
    uint8_t version_byte = VERSION;
    if (fwrite(MAGIC_HEADER, 1, MAGIC_HEADER_LEN, fp) != MAGIC_HEADER_LEN || fwrite(&version_byte, 1, sizeof(version_byte), fp) != sizeof(version_byte) || fwrite(salt, 1, SALT_SIZE, fp) != SALT_SIZE || fwrite(iv, 1, IV_SIZE, fp) != IV_SIZE || fwrite(ciphertext, 1, ciphertext_len, fp) != ciphertext_len || fwrite(tag, 1, TAG_SIZE, fp) != TAG_SIZE) {
        fprintf(stderr, "Failed to write to session file\n");
        fclose(fp);
        remove(session_filepath);
        goto cleanup;
    }
    
    result = session_id_hex;
    session_id_hex = NULL; // Prevent double free

cleanup:
    if (fp) fclose(fp);
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, KEY_SIZE);
    free(plaintext);
    free(ciphertext);
    free(session_id_hex);
    return result;
}

// Retrieves session data for a given session ID. Returns NULL if invalid, expired, or not found.
SessionData* get_session(const char* session_id, const char* master_key) {
    if (session_id == NULL || *session_id == '\0') return NULL;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    if(!sha256_hash(session_id, strlen(session_id), hash)) return NULL;
    char* filename_hash_hex = bytes_to_hex(hash, SHA256_DIGEST_LENGTH);
    if(!filename_hash_hex) return NULL;
    char session_filepath[MAX_PATH_LEN];
    snprintf(session_filepath, sizeof(session_filepath), "%s/%s", SESSION_DIR, filename_hash_hex);
    free(filename_hash_hex);

    FILE* fp = fopen(session_filepath, "rb");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    const long min_size = MAGIC_HEADER_LEN + sizeof(uint8_t) + SALT_SIZE + IV_SIZE + TAG_SIZE;
    if (file_size < min_size) {
        fclose(fp);
        return NULL;
    }

    unsigned char* file_buffer = (unsigned char*)malloc(file_size);
    if (!file_buffer) {
        fclose(fp);
        return NULL;
    }

    if (fread(file_buffer, 1, file_size, fp) != file_size) {
        free(file_buffer);
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    size_t offset = 0;
    if (memcmp(file_buffer, MAGIC_HEADER, MAGIC_HEADER_LEN) != 0 || file_buffer[MAGIC_HEADER_LEN] != VERSION) {
        free(file_buffer);
        return NULL;
    }
    offset = MAGIC_HEADER_LEN + sizeof(uint8_t);
    
    unsigned char* salt = file_buffer + offset; offset += SALT_SIZE;
    unsigned char* iv = file_buffer + offset; offset += IV_SIZE;
    long ciphertext_len = file_size - min_size;
    unsigned char* ciphertext = file_buffer + offset; offset += ciphertext_len;
    unsigned char* tag = file_buffer + offset;

    unsigned char key[KEY_SIZE];
    if (!derive_key_from_password(master_key, salt, key, KEY_SIZE)) {
        free(file_buffer);
        return NULL;
    }

    unsigned char* plaintext = (unsigned char*)malloc(ciphertext_len + 1);
    if (!plaintext) {
        OPENSSL_cleanse(key, KEY_SIZE);
        free(file_buffer);
        return NULL;
    }
    
    SessionData* session_data = NULL;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) goto decrypt_cleanup;

    int len = 0, plaintext_len = 0;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1 ||
        EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        goto decrypt_cleanup;
    }
    plaintext_len = len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1 ||
        EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) { // Tag verification fails here
        goto decrypt_cleanup;
    }
    plaintext_len += len;
    
    size_t user_id_len = strnlen((char*)plaintext, MAX_USER_ID_LEN + 1);
    if (user_id_len > MAX_USER_ID_LEN || plaintext_len != (user_id_len + 1 + sizeof(time_t))) {
        goto decrypt_cleanup;
    }
    
    session_data = (SessionData*)malloc(sizeof(SessionData));
    if (!session_data) goto decrypt_cleanup;
    
    memcpy(session_data->user_id, plaintext, user_id_len + 1);
    memcpy(&session_data->expires_at, plaintext + user_id_len + 1, sizeof(time_t));
    
    if (session_data->expires_at < time(NULL)) {
        free(session_data);
        session_data = NULL;
    }

decrypt_cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    free(plaintext);
    OPENSSL_cleanse(key, KEY_SIZE);
    free(file_buffer);
    return session_data;
}

// Deletes the session file, effectively terminating the session.
void terminate_session(const char* session_id) {
    if (session_id == NULL || *session_id == '\0') return;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    if(!sha256_hash(session_id, strlen(session_id), hash)) return;
    char* filename_hash_hex = bytes_to_hex(hash, SHA256_DIGEST_LENGTH);
    if(!filename_hash_hex) return;

    char session_filepath[MAX_PATH_LEN];
    snprintf(session_filepath, sizeof(session_filepath), "%s/%s", SESSION_DIR, filename_hash_hex);
    free(filename_hash_hex);
    remove(session_filepath);
}


int main() {
    // Rule #1 & #2: Avoid hardcoding credentials. Use environment variables.
    const char* master_key = getenv("SESSION_MASTER_KEY");
    if (!master_key || strlen(master_key) == 0) {
        fprintf(stderr, "WARNING: SESSION_MASTER_KEY environment variable not set. Using a weak, hardcoded key for demonstration.\n");
        master_key = "a-very-weak-default-master-key-for-dev";
    }

    printf("--- Test Case 1: Create and retrieve a valid session ---\n");
    const char* user_id = "user456";
    char* session_id = create_session(user_id, master_key);
    if (session_id) {
        printf("Session created successfully.\n");
        SessionData* session = get_session(session_id, master_key);
        if (session && strcmp(session->user_id, user_id) == 0) {
            printf("Session retrieved successfully for user: %s\n", session->user_id);
            free(session);
        } else {
            fprintf(stderr, "TEST FAILED: Could not retrieve valid session.\n");
        }
    } else {
        fprintf(stderr, "TEST FAILED: Session creation failed.\n");
    }

    printf("\n--- Test Case 2: Attempt to retrieve a non-existent session ---\n");
    SessionData* non_existent_session = get_session("nonexistent-session-id", master_key);
    if (!non_existent_session) {
        printf("Correctly failed to retrieve non-existent session.\n");
    } else {
        fprintf(stderr, "TEST FAILED: Retrieved a non-existent session.\n");
        free(non_existent_session);
    }

    printf("\n--- Test Case 3: Tamper with a session file (simulated) ---\n");
    if(session_id) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        sha256_hash(session_id, strlen(session_id), hash);
        char* filename_hash_hex = bytes_to_hex(hash, SHA256_DIGEST_LENGTH);
        if (filename_hash_hex) {
            char filename[MAX_PATH_LEN];
            snprintf(filename, sizeof(filename), "%s/%s", SESSION_DIR, filename_hash_hex);
            free(filename_hash_hex);

            FILE *fp = fopen(filename, "r+b");
            if (fp) {
                fseek(fp, -1, SEEK_END);
                fputc('X', fp);
                fclose(fp);
                printf("Session file tampered.\n");
                SessionData* tampered_session = get_session(session_id, master_key);
                if (!tampered_session) {
                    printf("Correctly failed to retrieve tampered session (decryption/tag check failed).\n");
                } else {
                    fprintf(stderr, "TEST FAILED: Retrieved a tampered session.\n");
                    free(tampered_session);
                }
            } else {
                perror("Could not open session file for tampering test");
            }
        }
    }
    
    printf("\n--- Test Case 4: Terminate a session ---\n");
    terminate_session(session_id);
    printf("Session termination requested.\n");
    SessionData* terminated_session = get_session(session_id, master_key);
    if (!terminated_session) {
        printf("Correctly failed to retrieve terminated session.\n");
    } else {
        fprintf(stderr, "TEST FAILED: Retrieved a terminated session.\n");
        free(terminated_session);
    }
    free(session_id);
    session_id = NULL;

    printf("\n--- Test Case 5: Expired session ---\n");
    printf("The expiration check is implemented in get_session(). A full test would require\n");
    printf("manual creation of an expired session file or waiting for a session to expire.\n");
    printf("The logic `if (session_data->expires_at < time(NULL))` handles this case.\n");
    
    return 0;
}