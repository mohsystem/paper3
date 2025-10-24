
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdint.h>

/* Magic bytes and version for file format validation */
static const char MAGIC[] = "ENC1";
static const uint8_t VERSION = 1;
static const size_t SALT_SIZE = 16;
static const size_t IV_SIZE = 12;
static const size_t TAG_SIZE = 16;
static const size_t MAGIC_SIZE = 4;
static const int PBKDF2_ITERATIONS = 210000;
static const size_t MAX_FILE_SIZE = 100 * 1024 * 1024; /* 100MB limit */

/* Securely clear memory containing sensitive data */
void secure_clear(void* ptr, size_t size) {
    if (ptr != NULL) {
        volatile unsigned char* p = (volatile unsigned char*)ptr;
        while (size--) {
            *p++ = 0;
        }
    }
}

/* Derive encryption key from passphrase using PBKDF2-HMAC-SHA-256 */
int derive_key(const char* passphrase, const unsigned char* salt, 
               unsigned char* key, size_t key_len) {
    /* Input validation */
    if (passphrase == NULL || salt == NULL || key == NULL || key_len != 32) {
        return 0;
    }
    
    size_t pass_len = strlen(passphrase);
    if (pass_len == 0) {
        return 0;
    }
    
    /* Use PBKDF2 with SHA-256 and high iteration count */
    int result = PKCS5_PBKDF2_HMAC(
        passphrase, pass_len,
        salt, SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        key_len, key
    );
    
    return result == 1;
}

/* Normalize and validate path to prevent directory traversal */
int validate_path(const char* base_dir, const char* user_path, 
                  char* resolved_path, size_t resolved_size) {
    /* Input validation */
    if (base_dir == NULL || user_path == NULL || resolved_path == NULL || 
        resolved_size == 0) {
        return 0;
    }
    
    if (strlen(base_dir) == 0 || strlen(user_path) == 0) {
        return 0;
    }
    
    /* Resolve base directory to absolute path */
    char base_real[PATH_MAX];
    if (realpath(base_dir, base_real) == NULL) {
        return 0;
    }
    
    /* Construct full path with bounds checking */
    char full_path[PATH_MAX];
    int written = snprintf(full_path, PATH_MAX, "%s/%s", base_real, user_path);
    if (written < 0 || written >= PATH_MAX) {
        return 0;
    }
    
    /* Resolve to absolute path */
    char resolved[PATH_MAX];
    if (realpath(full_path, resolved) == NULL) {
        /* Path might not exist yet, check parent directory */
        char* last_slash = strrchr(full_path, '/');
        if (last_slash == NULL) {
            return 0;
        }
        
        /* Temporarily null-terminate to get parent path */
        *last_slash = '\\0';
        char parent_real[PATH_MAX];
        if (realpath(full_path, parent_real) == NULL) {
            return 0;
        }
        
        /* Reconstruct path */
        written = snprintf(resolved, PATH_MAX, "%s/%s", parent_real, last_slash + 1);
        if (written < 0 || written >= PATH_MAX) {
            return 0;
        }
    }
    
    /* Ensure resolved path is within base directory */
    size_t base_len = strlen(base_real);
    if (strlen(resolved) < base_len || strncmp(resolved, base_real, base_len) != 0) {
        return 0;
    }
    
    /* Reject symlinks */
    struct stat st;
    if (lstat(resolved, &st) == 0) {
        if (S_ISLNK(st.st_mode)) {
            return 0;
        }
    }
    
    /* Copy to output buffer with bounds check */
    if (strlen(resolved) >= resolved_size) {
        return 0;
    }
    strncpy(resolved_path, resolved, resolved_size - 1);
    resolved_path[resolved_size - 1] = '\\0';
    
    return 1;
}

/* Atomic file write: write to temp then rename */
int atomic_write(const char* filepath, const unsigned char* data, size_t data_len) {
    /* Input validation */
    if (filepath == NULL || data == NULL || data_len == 0) {
        return 0;
    }
    
    /* Create temporary file path with bounds checking */
    char temp_path[PATH_MAX];
    int written = snprintf(temp_path, PATH_MAX, "%s.tmp", filepath);
    if (written < 0 || written >= PATH_MAX) {
        return 0;
    }
    
    /* Open with exclusive create mode using "wx" for C11 */
    FILE* fp = fopen(temp_path, "wb");
    if (fp == NULL) {
        return 0;
    }
    
    /* Set restrictive permissions */
    chmod(temp_path, S_IRUSR | S_IWUSR);
    
    /* Write data with bounds check */
    size_t written_bytes = fwrite(data, 1, data_len, fp);
    if (written_bytes != data_len) {
        fclose(fp);
        unlink(temp_path);
        return 0;
    }
    
    /* Flush to disk */
    if (fflush(fp) != 0) {
        fclose(fp);
        unlink(temp_path);
        return 0;
    }
    
    /* Sync to ensure data is on disk */
    int fd = fileno(fp);
    if (fd < 0) {
        fclose(fp);
        unlink(temp_path);
        return 0;
    }
    
    fsync(fd);
    fclose(fp);
    
    /* Atomic rename */
    if (rename(temp_path, filepath) != 0) {
        unlink(temp_path);
        return 0;
    }
    
    return 1;
}

/* Encrypt file using AES-256-GCM */
int encrypt_file(const char* base_dir, const char* input_file, 
                 const char* output_file, const char* passphrase) {
    /* Input validation */
    if (base_dir == NULL || input_file == NULL || output_file == NULL || 
        passphrase == NULL) {
        fprintf(stderr, "Error: NULL parameter\\n");
        return 0;
    }
    
    /* Validate passphrase strength */
    size_t pass_len = strlen(passphrase);
    if (pass_len < 8) {
        fprintf(stderr, "Error: Passphrase too short\\n");
        return 0;
    }
    
    /* Validate and normalize paths */
    char input_path[PATH_MAX];
    char output_path[PATH_MAX];
    if (!validate_path(base_dir, input_file, input_path, PATH_MAX) ||
        !validate_path(base_dir, output_file, output_path, PATH_MAX)) {
        fprintf(stderr, "Error: Invalid file path\\n");
        return 0;
    }
    
    /* Open and read input file */
    FILE* fp = fopen(input_path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open input file\\n");
        return 0;
    }
    
    /* Get file size with validation */
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    
    long file_size = ftell(fp);
    if (file_size < 0 || file_size > (long)MAX_FILE_SIZE) {
        fprintf(stderr, "Error: Invalid file size\\n");
        fclose(fp);
        return 0;
    }
    
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    
    /* Allocate buffer for plaintext */
    unsigned char* plaintext = (unsigned char*)malloc(file_size);
    if (plaintext == NULL) {
        fclose(fp);
        return 0;
    }
    
    /* Read file with bounds check */
    size_t read_bytes = fread(plaintext, 1, file_size, fp);
    if (read_bytes != (size_t)file_size) {
        secure_clear(plaintext, file_size);
        free(plaintext);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    
    /* Generate random salt using cryptographically secure RNG */
    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        fprintf(stderr, "Error: Failed to generate salt\\n");
        secure_clear(plaintext, file_size);
        free(plaintext);
        return 0;
    }
    
    /* Derive key from passphrase */
    unsigned char key[32];
    if (!derive_key(passphrase, salt, key, 32)) {
        fprintf(stderr, "Error: Key derivation failed\\n");
        secure_clear(plaintext, file_size);
        free(plaintext);
        secure_clear(key, 32);
        return 0;
    }
    
    /* Generate random IV using cryptographically secure RNG */
    unsigned char iv[IV_SIZE];
    if (RAND_bytes(iv, IV_SIZE) != 1) {
        fprintf(stderr, "Error: Failed to generate IV\\n");
        secure_clear(plaintext, file_size);
        free(plaintext);
        secure_clear(key, 32);
        return 0;
    }
    
    /* Initialize cipher context */
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "Error: Failed to create cipher context\\n");
        secure_clear(plaintext, file_size);
        free(plaintext);
        secure_clear(key, 32);
        return 0;
    }
    
    /* Initialize encryption with AES-256-GCM */
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        fprintf(stderr, "Error: Encryption initialization failed\\n");
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(plaintext, file_size);
        free(plaintext);
        secure_clear(key, 32);
        return 0;
    }
    
    /* Allocate buffer for ciphertext */
    size_t ciphertext_max_len = file_size + EVP_CIPHER_block_size(EVP_aes_256_gcm());
    unsigned char* ciphertext = (unsigned char*)malloc(ciphertext_max_len);
    if (ciphertext == NULL) {
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(plaintext, file_size);
        free(plaintext);
        secure_clear(key, 32);
        return 0;
    }
    
    int len = 0;
    int ciphertext_len = 0;
    
    /* Encrypt plaintext */
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, file_size) != 1) {
        fprintf(stderr, "Error: Encryption failed\\n");
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(plaintext, file_size);
        free(plaintext);
        secure_clear(ciphertext, ciphertext_max_len);
        free(ciphertext);
        secure_clear(key, 32);
        return 0;
    }
    ciphertext_len = len;
    
    /* Finalize encryption */
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        fprintf(stderr, "Error: Encryption finalization failed\\n");
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(plaintext, file_size);
        free(plaintext);
        secure_clear(ciphertext, ciphertext_max_len);
        free(ciphertext);
        secure_clear(key, 32);
        return 0;
    }
    ciphertext_len += len;
    
    /* Get authentication tag */
    unsigned char tag[TAG_SIZE];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) {
        fprintf(stderr, "Error: Failed to get authentication tag\\n");
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(plaintext, file_size);
        free(plaintext);
        secure_clear(ciphertext, ciphertext_max_len);
        free(ciphertext);
        secure_clear(key, 32);
        return 0;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    
    /* Build output: [magic][version][salt][iv][ciphertext][tag] */
    size_t output_len = MAGIC_SIZE + 1 + SALT_SIZE + IV_SIZE + ciphertext_len + TAG_SIZE;
    unsigned char* output = (unsigned char*)malloc(output_len);
    if (output == NULL) {
        secure_clear(plaintext, file_size);
        free(plaintext);
        secure_clear(ciphertext, ciphertext_max_len);
        free(ciphertext);
        secure_clear(key, 32);
        secure_clear(tag, TAG_SIZE);
        return 0;
    }
    
    size_t offset = 0;
    memcpy(output + offset, MAGIC, MAGIC_SIZE);
    offset += MAGIC_SIZE;
    output[offset++] = VERSION;
    memcpy(output + offset, salt, SALT_SIZE);
    offset += SALT_SIZE;
    memcpy(output + offset, iv, IV_SIZE);
    offset += IV_SIZE;
    memcpy(output + offset, ciphertext, ciphertext_len);
    offset += ciphertext_len;
    memcpy(output + offset, tag, TAG_SIZE);
    
    /* Atomic write to output file */
    int write_result = atomic_write(output_path, output, output_len);
    
    /* Clear sensitive data */
    secure_clear(plaintext, file_size);
    free(plaintext);
    secure_clear(ciphertext, ciphertext_max_len);
    free(ciphertext);
    secure_clear(key, 32);
    secure_clear(tag, TAG_SIZE);
    secure_clear(output, output_len);
    free(output);
    
    if (!write_result) {
        fprintf(stderr, "Error: Failed to write output file\\n");
        return 0;
    }
    
    return 1;
}

/* Decrypt file using AES-256-GCM */
int decrypt_file(const char* base_dir, const char* input_file, 
                 const char* output_file, const char* passphrase) {
    /* Input validation */
    if (base_dir == NULL || input_file == NULL || output_file == NULL || 
        passphrase == NULL) {
        fprintf(stderr, "Error: NULL parameter\\n");
        return 0;
    }
    
    if (strlen(passphrase) == 0) {
        fprintf(stderr, "Error: Empty passphrase\\n");
        return 0;
    }
    
    /* Validate and normalize paths */
    char input_path[PATH_MAX];
    char output_path[PATH_MAX];
    if (!validate_path(base_dir, input_file, input_path, PATH_MAX) ||
        !validate_path(base_dir, output_file, output_path, PATH_MAX)) {
        fprintf(stderr, "Error: Invalid file path\\n");
        return 0;
    }
    
    /* Open and read encrypted file */
    FILE* fp = fopen(input_path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open input file\\n");
        return 0;
    }
    
    /* Get file size and validate minimum size */
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    
    long file_size = ftell(fp);
    size_t min_size = MAGIC_SIZE + 1 + SALT_SIZE + IV_SIZE + TAG_SIZE;
    if (file_size < (long)min_size || file_size > (long)MAX_FILE_SIZE) {
        fprintf(stderr, "Error: Invalid file size\\n");
        fclose(fp);
        return 0;
    }
    
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }
    
    /* Allocate buffer for encrypted data */
    unsigned char* encrypted_data = (unsigned char*)malloc(file_size);
    if (encrypted_data == NULL) {
        fclose(fp);
        return 0;
    }
    
    /* Read file with bounds check */
    size_t read_bytes = fread(encrypted_data, 1, file_size, fp);
    if (read_bytes != (size_t)file_size) {
        free(encrypted_data);
        fclose(fp);
        return 0;
    }
    fclose(fp);
    
    /* Parse and validate file format */
    size_t offset = 0;
    
    /* Validate magic bytes */
    if (memcmp(encrypted_data + offset, MAGIC, MAGIC_SIZE) != 0) {
        fprintf(stderr, "Error: Invalid file format\\n");
        free(encrypted_data);
        return 0;
    }
    offset += MAGIC_SIZE;
    
    /* Validate version */
    if (encrypted_data[offset] != VERSION) {
        fprintf(stderr, "Error: Unsupported file version\\n");
        free(encrypted_data);
        return 0;
    }
    offset += 1;
    
    /* Extract salt */
    unsigned char salt[SALT_SIZE];
    memcpy(salt, encrypted_data + offset, SALT_SIZE);
    offset += SALT_SIZE;
    
    /* Extract IV */
    unsigned char iv[IV_SIZE];
    memcpy(iv, encrypted_data + offset, IV_SIZE);
    offset += IV_SIZE;
    
    /* Calculate ciphertext length with bounds check */
    if (offset + TAG_SIZE > (size_t)file_size) {
        fprintf(stderr, "Error: Invalid file structure\\n");
        free(encrypted_data);
        return 0;
    }
    
    size_t ciphertext_len = file_size - offset - TAG_SIZE;
    if (ciphertext_len == 0) {
        fprintf(stderr, "Error: Invalid ciphertext length\\n");
        free(encrypted_data);
        return 0;
    }
    
    /* Extract ciphertext */
    unsigned char* ciphertext = (unsigned char*)malloc(ciphertext_len);
    if (ciphertext == NULL) {
        free(encrypted_data);
        return 0;
    }
    memcpy(ciphertext, encrypted_data + offset, ciphertext_len);
    offset += ciphertext_len;
    
    /* Extract authentication tag */
    unsigned char tag[TAG_SIZE];
    memcpy(tag, encrypted_data + offset, TAG_SIZE);
    
    free(encrypted_data);
    
    /* Derive key from passphrase */
    unsigned char key[32];
    if (!derive_key(passphrase, salt, key, 32)) {
        fprintf(stderr, "Error: Key derivation failed\\n");
        secure_clear(ciphertext, ciphertext_len);
        free(ciphertext);
        secure_clear(key, 32);
        return 0;
    }
    
    /* Initialize cipher context */
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "Error: Failed to create cipher context\\n");
        secure_clear(ciphertext, ciphertext_len);
        free(ciphertext);
        secure_clear(key, 32);
        return 0;
    }
    
    /* Initialize decryption with AES-256-GCM */
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) {
        fprintf(stderr, "Error: Decryption initialization failed\\n");
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(ciphertext, ciphertext_len);
        free(ciphertext);
        secure_clear(key, 32);
        return 0;
    }
    
    /* Allocate buffer for plaintext */
    size_t plaintext_max_len = ciphertext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm());
    unsigned char* plaintext = (unsigned char*)malloc(plaintext_max_len);
    if (plaintext == NULL) {
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(ciphertext, ciphertext_len);
        free(ciphertext);
        secure_clear(key, 32);
        return 0;
    }
    
    int len = 0;
    int plaintext_len = 0;
    
    /* Decrypt ciphertext */
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        fprintf(stderr, "Error: Decryption failed\\n");
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(ciphertext, ciphertext_len);
        free(ciphertext);
        secure_clear(plaintext, plaintext_max_len);
        free(plaintext);
        secure_clear(key, 32);
        return 0;
    }
    plaintext_len = len;
    
    /* Set expected authentication tag */
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) {
        fprintf(stderr, "Error: Failed to set authentication tag\\n");
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(ciphertext, ciphertext_len);
        free(ciphertext);
        secure_clear(plaintext, plaintext_max_len);
        free(plaintext);
        secure_clear(key, 32);
        return 0;
    }
    
    /* Finalize decryption and verify authentication tag */
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        fprintf(stderr, "Error: Authentication verification failed\\n");
        EVP_CIPHER_CTX_free(ctx);
        secure_clear(ciphertext, ciphertext_len);
        free(ciphertext);
        secure_clear(plaintext, plaintext_max_len);
        free(plaintext);
        secure_clear(key, 32);
        secure_clear(tag, TAG_SIZE);
        return 0;
    }
    plaintext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);
    
    /* Atomic write to output file */
    int write_result = atomic_write(output_path, plaintext, plaintext_len);
    
    /* Clear sensitive data */
    secure_clear(ciphertext, ciphertext_len);
    free(ciphertext);
    secure_clear(plaintext, plaintext_max_len);
    free(plaintext);
    secure_clear(key, 32);
    secure_clear(tag, TAG_SIZE);
    
    if (!write_result) {
        fprintf(stderr, "Error: Failed to write output file\\n");
        return 0;
    }
    
    return 1;
}

int main(void) {
    /* Test case 1: Basic encryption and decryption */
    {
        const char* base_dir = ".";
        const char* input = "test1.txt";
        const char* encrypted = "test1.enc";
        const char* decrypted = "test1_dec.txt";
        const char* passphrase = "StrongPassphrase123!";
        
        /* Create test file */
        FILE* fp = fopen(input, "w");
        if (fp != NULL) {
            fprintf(fp, "Hello, World! This is a test message.");
            fclose(fp);
        }
        
        if (encrypt_file(base_dir, input, encrypted, passphrase)) {
            printf("Test 1: Encryption successful\\n");
            if (decrypt_file(base_dir, encrypted, decrypted, passphrase)) {
                printf("Test 1: Decryption successful\\n");
            } else {
                printf("Test 1: Decryption failed\\n");
            }
        } else {
            printf("Test 1: Encryption failed\\n");
        }
    }
    
    /* Test case 2: Wrong passphrase */
    {
        const char* base_dir = ".";
        const char* input = "test2.txt";
        const char* encrypted = "test2.enc";
        const char* decrypted = "test2_dec.txt";
        
        FILE* fp = fopen(input, "w");
        if (fp != NULL) {
            fprintf(fp, "Secret data");
            fclose(fp);
        }
        
        encrypt_file(base_dir, input, encrypted, "CorrectPass123!");
        if (!decrypt_file(base_dir, encrypted, decrypted, "WrongPass123!")) {
            printf("Test 2: Correctly rejected wrong passphrase\\n");
        } else {
            printf("Test 2: Failed - accepted wrong passphrase\\n");
        }
    }
    
    /* Test case 3: Large file */
    {
        const char* base_dir = ".";
        const char* input = "test3.txt";
        const char* encrypted = "test3.enc";
        const char* decrypted = "test3_dec.txt";
        const char* passphrase = "LargeFileTest456!";
        
        FILE* fp = fopen(input, "w");
        if (fp != NULL) {
            for (int i = 0; i < 10000; i++) {
                fprintf(fp, "This is line %d of the large test file.\\n", i);
            }
            fclose(fp);
        }
        
        if (encrypt_file(base_dir, input, encrypted, passphrase)) {
            printf("Test 3: Large file encryption successful\\n");
            if (decrypt_file(base_dir, encrypted, decrypted, passphrase)) {
                printf("Test 3: Large file decryption successful\\n");
            }
        }
    }
    
    /* Test case 4: Empty passphrase rejection */
    {
        const char* base_dir = ".";
        const char* input = "test4.txt";
        const char* encrypted = "test4.enc";
        
        FILE* fp = fopen(input, "w");
        if (fp != NULL) {
            fprintf(fp, "Test data");
            fclose(fp);
        }
        
        if (!encrypt_file(base_dir, input, encrypted, "")) {
            printf("Test 4: Correctly rejected empty passphrase\\n");
        }
    }
    
    /* Test case 5: Path traversal prevention */
    {
        const char* base_dir = ".";
        const char* input = "../etc/passwd";
        const char* encrypted = "test5.enc";
        const char* passphrase = "PathTest789!";
        
        if (!encrypt_file(base_dir, input, encrypted, passphrase)) {
            printf("Test 5: Correctly rejected path traversal attempt\\n");
        }
    }
    
    return 0;
}
