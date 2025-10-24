#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/kdf.h>

// To compile: gcc your_file.c -o your_app -lssl -lcrypto

// Constants based on security requirements
#define KEY_SIZE 32       // 256 bits for AES-256
#define IV_SIZE 12        // 96 bits for GCM, a standard size
#define SALT_SIZE 16      // 128 bits
#define TAG_SIZE 16       // 128 bits for GCM
#define PBKDF2_ITER 310000 // Recommended minimum is increasing

// Helper to print OpenSSL errors
void handle_openssl_errors() {
    unsigned long err_code;
    while ((err_code = ERR_get_error())) {
        char err_msg[256];
        ERR_error_string_n(err_code, err_msg, sizeof(err_msg));
        fprintf(stderr, "OpenSSL Error: %s\n", err_msg);
    }
}

// Securely clears memory before freeing
void secure_free(void* ptr, size_t len) {
    if (ptr) {
        OPENSSL_cleanse(ptr, len);
        free(ptr);
    }
}

int encrypt_file(const char* input_path, const char* output_path, const char* password) {
    FILE* in_file = NULL;
    FILE* out_file = NULL;
    unsigned char* plaintext = NULL;
    long plaintext_len = 0;
    unsigned char* ciphertext = NULL;
    unsigned char* key = NULL;
    EVP_CIPHER_CTX* ctx = NULL;
    char temp_output_path[256];
    int ret = -1;

    in_file = fopen(input_path, "rb");
    if (!in_file) {
        perror("Cannot open input file");
        goto cleanup;
    }

    fseek(in_file, 0, SEEK_END);
    plaintext_len = ftell(in_file);
    if (plaintext_len < 0) {
        perror("ftell failed");
        goto cleanup;
    }
    fseek(in_file, 0, SEEK_SET);

    plaintext = malloc(plaintext_len > 0 ? plaintext_len : 1);
    if (!plaintext) {
        perror("malloc plaintext failed");
        goto cleanup;
    }
    if (fread(plaintext, 1, plaintext_len, in_file) != (size_t)plaintext_len) {
        fprintf(stderr, "Failed to read plaintext from file.\n");
        goto cleanup;
    }
    fclose(in_file);
    in_file = NULL;

    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        handle_openssl_errors();
        goto cleanup;
    }

    key = malloc(KEY_SIZE);
    if (!key) { perror("malloc key failed"); goto cleanup; }
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), PBKDF2_ITER, EVP_sha256(), KEY_SIZE, key) != 1) {
        handle_openssl_errors();
        goto cleanup;
    }
    
    unsigned char iv[IV_SIZE];
    if (RAND_bytes(iv, sizeof(iv)) != 1) { handle_openssl_errors(); goto cleanup; }

    ciphertext = malloc(plaintext_len + TAG_SIZE); // GCM can be same size, but add block for safety
    if (!ciphertext) { perror("malloc ciphertext failed"); goto cleanup; }
    
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { handle_openssl_errors(); goto cleanup; }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) { handle_openssl_errors(); goto cleanup; }

    int len = 0;
    int ciphertext_len = 0;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) { handle_openssl_errors(); goto cleanup; }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) { handle_openssl_errors(); goto cleanup; }
    ciphertext_len += len;

    unsigned char tag[TAG_SIZE];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag) != 1) { handle_openssl_errors(); goto cleanup; }

    snprintf(temp_output_path, sizeof(temp_output_path), "%s.tmp", output_path);
    out_file = fopen(temp_output_path, "wb");
    if (!out_file) { perror("Cannot create temporary output file"); goto cleanup; }

    if (fwrite(salt, 1, sizeof(salt), out_file) != sizeof(salt) ||
        fwrite(iv, 1, sizeof(iv), out_file) != sizeof(iv) ||
        fwrite(ciphertext, 1, ciphertext_len, out_file) != (size_t)ciphertext_len ||
        fwrite(tag, 1, sizeof(tag), out_file) != sizeof(tag)) {
        fprintf(stderr, "Failed to write to temporary file.\n");
        goto cleanup;
    }
    fclose(out_file);
    out_file = NULL;

    if (rename(temp_output_path, output_path) != 0) {
        perror("Failed to rename temporary file");
        remove(temp_output_path);
        goto cleanup;
    }

    ret = 0; // Success

cleanup:
    if (in_file) fclose(in_file);
    if (out_file) { fclose(out_file); remove(temp_output_path); }
    secure_free(plaintext, plaintext_len > 0 ? plaintext_len : 1);
    secure_free(ciphertext, plaintext_len > 0 ? plaintext_len + TAG_SIZE : 1 + TAG_SIZE);
    secure_free(key, KEY_SIZE);
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    return ret;
}

int decrypt_file(const char* input_path, const char* output_path, const char* password) {
    FILE* in_file = NULL;
    FILE* out_file = NULL;
    unsigned char* encrypted_data = NULL;
    unsigned char* decrypted_text = NULL;
    unsigned char* key = NULL;
    EVP_CIPHER_CTX* ctx = NULL;
    char temp_output_path[256];
    long encrypted_len = 0;
    long ciphertext_len = 0;
    int ret = -1;

    in_file = fopen(input_path, "rb");
    if (!in_file) { perror("Cannot open input file"); goto cleanup; }

    fseek(in_file, 0, SEEK_END);
    encrypted_len = ftell(in_file);
    if (encrypted_len < SALT_SIZE + IV_SIZE + TAG_SIZE) {
        fprintf(stderr, "Invalid encrypted file size.\n");
        goto cleanup;
    }
    fseek(in_file, 0, SEEK_SET);
    
    encrypted_data = malloc(encrypted_len);
    if (!encrypted_data) { perror("malloc encrypted_data failed"); goto cleanup; }
    if (fread(encrypted_data, 1, encrypted_len, in_file) != (size_t)encrypted_len) {
        fprintf(stderr, "Failed to read encrypted data.\n");
        goto cleanup;
    }
    fclose(in_file);
    in_file = NULL;

    unsigned char* salt = encrypted_data;
    unsigned char* iv = encrypted_data + SALT_SIZE;
    unsigned char* ciphertext = encrypted_data + SALT_SIZE + IV_SIZE;
    ciphertext_len = encrypted_len - SALT_SIZE - IV_SIZE - TAG_SIZE;
    unsigned char* tag = encrypted_data + SALT_SIZE + IV_SIZE + ciphertext_len;

    key = malloc(KEY_SIZE);
    if (!key) { perror("malloc key failed"); goto cleanup; }
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_SIZE, PBKDF2_ITER, EVP_sha256(), KEY_SIZE, key) != 1) {
        handle_openssl_errors();
        goto cleanup;
    }

    decrypted_text = malloc(ciphertext_len > 0 ? ciphertext_len : 1);
    if (!decrypted_text) { perror("malloc decrypted_text failed"); goto cleanup; }

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { handle_openssl_errors(); goto cleanup; }
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1) { handle_openssl_errors(); goto cleanup; }

    int len = 0;
    int decrypted_len = 0;
    if (EVP_DecryptUpdate(ctx, decrypted_text, &len, ciphertext, ciphertext_len) != 1) {
        handle_openssl_errors();
        goto cleanup;
    }
    decrypted_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag) != 1) { handle_openssl_errors(); goto cleanup; }

    if (EVP_DecryptFinal_ex(ctx, decrypted_text + len, &len) <= 0) {
        fprintf(stderr, "Decryption failed. Data is corrupt or password is wrong.\n");
        goto cleanup;
    }
    decrypted_len += len;

    snprintf(temp_output_path, sizeof(temp_output_path), "%s.tmp", output_path);
    out_file = fopen(temp_output_path, "wb");
    if (!out_file) { perror("Cannot create temporary output file"); goto cleanup; }
    if (fwrite(decrypted_text, 1, decrypted_len, out_file) != (size_t)decrypted_len) {
        fprintf(stderr, "Failed to write decrypted data.\n");
        goto cleanup;
    }
    fclose(out_file);
    out_file = NULL;

    if (rename(temp_output_path, output_path) != 0) {
        perror("Failed to rename temporary file");
        remove(temp_output_path);
        goto cleanup;
    }

    ret = 0; // Success

cleanup:
    if (in_file) fclose(in_file);
    if (out_file) { fclose(out_file); remove(temp_output_path); }
    secure_free(encrypted_data, encrypted_len);
    secure_free(decrypted_text, ciphertext_len > 0 ? ciphertext_len : 1);
    secure_free(key, KEY_SIZE);
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    return ret;
}

// Test helpers
void create_file(const char* path, const char* content, size_t content_len) {
    FILE* f = fopen(path, "wb");
    if (f) {
        if (content_len > 0) {
            fwrite(content, 1, content_len, f);
        }
        fclose(f);
    }
}

int compare_files(const char* path1, const char* path2) {
    FILE* f1 = fopen(path1, "rb");
    FILE* f2 = fopen(path2, "rb");
    if (!f1 || !f2) { if (f1) fclose(f1); if (f2) fclose(f2); return 0; }
    fseek(f1, 0, SEEK_END); fseek(f2, 0, SEEK_END);
    long size1 = ftell(f1); long size2 = ftell(f2);
    if (size1 != size2) { fclose(f1); fclose(f2); return 0; }
    fseek(f1, 0, SEEK_SET); fseek(f2, 0, SEEK_SET);
    int result = 1; int ch1, ch2;
    while ((ch1 = fgetc(f1)) != EOF) {
        ch2 = fgetc(f2);
        if (ch1 != ch2) { result = 0; break; }
    }
    fclose(f1); fclose(f2);
    return result;
}


int main() {
    const char* password = "a_very_strong_password_123!@#";

    printf("--- Running C Test Cases ---\n");

    // Test Case 1: Encrypt and decrypt a simple text file
    printf("Test Case 1: Simple text file encryption/decryption... ");
    const char* msg1 = "Hello, this is a secret message.";
    create_file("ctest1.txt", msg1, strlen(msg1));
    if (encrypt_file("ctest1.txt", "ctest1.enc", password) == 0 &&
        decrypt_file("ctest1.enc", "ctest1.dec", password) == 0 &&
        compare_files("ctest1.txt", "ctest1.dec")) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }

    // Test Case 2: Decryption with wrong password
    printf("Test Case 2: Decryption with wrong password... ");
    if (decrypt_file("ctest1.enc", "ctest2.dec", "wrongpassword") != 0) {
        printf("PASSED (Decryption failed as expected)\n");
    } else {
        printf("FAILED (Decryption succeeded unexpectedly)\n");
    }

    // Test Case 3: Encrypt and decrypt an empty file
    printf("Test Case 3: Empty file... ");
    create_file("cempty.txt", "", 0);
    if (encrypt_file("cempty.txt", "cempty.enc", password) == 0 &&
        decrypt_file("cempty.enc", "cempty.dec", password) == 0 &&
        compare_files("cempty.txt", "cempty.dec")) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
    
    // Test Case 4: File with binary data (a few non-text bytes)
    printf("Test Case 4: Binary data... ");
    char binary_data[] = {'\x01', '\x02', '\x00', '\xFE', '\xFF', '\x80'};
    create_file("cbinary.dat", binary_data, sizeof(binary_data));
    if (encrypt_file("cbinary.dat", "cbinary.enc", password) == 0 &&
        decrypt_file("cbinary.enc", "cbinary.dec", password) == 0 &&
        compare_files("cbinary.dat", "cbinary.dec")) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }
    
    // Test Case 5: A slightly larger file (10KB)
    printf("Test Case 5: Larger file (10KB)... ");
    char* large_content = malloc(10 * 1024);
    if(large_content) {
        memset(large_content, 'X', 10 * 1024);
        create_file("clarge.txt", large_content, 10 * 1024);
        free(large_content);
    }
    if (encrypt_file("clarge.txt", "clarge.enc", password) == 0 &&
        decrypt_file("clarge.enc", "clarge.dec", password) == 0 &&
        compare_files("clarge.txt", "clarge.dec")) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }

    // Cleanup
    remove("ctest1.txt"); remove("ctest1.enc"); remove("ctest1.dec");
    remove("ctest2.dec");
    remove("cempty.txt"); remove("cempty.enc"); remove("cempty.dec");
    remove("cbinary.dat"); remove("cbinary.enc"); remove("cbinary.dec");
    remove("clarge.txt"); remove("clarge.enc"); remove("clarge.dec");
    
    return 0;
}