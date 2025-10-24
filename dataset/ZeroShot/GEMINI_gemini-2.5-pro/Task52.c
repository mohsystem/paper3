// Note: This code requires the OpenSSL library.
// Compile with: gcc -o your_program_name your_source_file.c -lssl -lcrypto
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Constants
#define KEY_SIZE 32 // 256-bit
#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define PBKDF2_ITERATIONS 65536
#define BUFFER_SIZE 4096

void handle_openssl_errors() {
    // In a real app, log this properly
    // ERR_print_errors_fp(stderr);
}

int getKeyFromPassword(const char* password, const unsigned char* salt, unsigned char* key) {
    return PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_SIZE, PBKDF2_ITERATIONS, EVP_sha256(), KEY_SIZE, key);
}

int encryptFile(const char* inputFile, const char* outputFile, const char* password) {
    FILE* inFile = fopen(inputFile, "rb");
    if (!inFile) return 0;
    FILE* outFile = fopen(outputFile, "wb");
    if (!outFile) { fclose(inFile); return 0; }

    unsigned char salt[SALT_SIZE];
    if (!RAND_bytes(salt, sizeof(salt))) {
        fclose(inFile); fclose(outFile); return 0;
    }

    unsigned char key[KEY_SIZE];
    if (!getKeyFromPassword(password, salt, key)) {
        fclose(inFile); fclose(outFile); return 0;
    }

    unsigned char iv[IV_SIZE];
    if (!RAND_bytes(iv, sizeof(iv))) {
        fclose(inFile); fclose(outFile); return 0;
    }

    fwrite(salt, 1, sizeof(salt), outFile);
    fwrite(iv, 1, sizeof(iv), outFile);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { fclose(inFile); fclose(outFile); return 0; }
    
    int success = 1;
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) success = 0;
    if (success && 1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL)) success = 0;
    if (success && 1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) success = 0;

    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char outBuffer[BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH];
    int len = 0;
    int bytesRead = 0;

    if (success) {
        while ((bytesRead = fread(inBuffer, 1, BUFFER_SIZE, inFile)) > 0) {
            if (1 != EVP_EncryptUpdate(ctx, outBuffer, &len, inBuffer, bytesRead)) {
                success = 0;
                break;
            }
            fwrite(outBuffer, 1, len, outFile);
        }
    }

    if (success && 1 != EVP_EncryptFinal_ex(ctx, outBuffer, &len)) success = 0;
    if (success) fwrite(outBuffer, 1, len, outFile);

    unsigned char tag[TAG_SIZE];
    if (success && 1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag)) success = 0;
    if (success) fwrite(tag, 1, sizeof(tag), outFile);

    EVP_CIPHER_CTX_free(ctx);
    fclose(inFile);
    fclose(outFile);

    if (!success) { remove(outputFile); }
    return success;
}

int decryptFile(const char* inputFile, const char* outputFile, const char* password) {
    FILE* inFile = fopen(inputFile, "rb");
    if (!inFile) return 0;
    FILE* outFile = fopen(outputFile, "wb");
    if (!outFile) { fclose(inFile); return 0; }

    unsigned char salt[SALT_SIZE];
    if (fread(salt, 1, sizeof(salt), inFile) != sizeof(salt)) {
        fclose(inFile); fclose(outFile); remove(outputFile); return 0;
    }

    unsigned char iv[IV_SIZE];
    if (fread(iv, 1, sizeof(iv), inFile) != sizeof(iv)) {
        fclose(inFile); fclose(outFile); remove(outputFile); return 0;
    }

    fseek(inFile, -TAG_SIZE, SEEK_END);
    unsigned char tag[TAG_SIZE];
    if (fread(tag, 1, sizeof(tag), inFile) != sizeof(tag)) {
        fclose(inFile); fclose(outFile); remove(outputFile); return 0;
    }
    
    long ciphertext_len = ftell(inFile) - (SALT_SIZE + IV_SIZE + TAG_SIZE);
    fseek(inFile, SALT_SIZE + IV_SIZE, SEEK_SET);

    unsigned char key[KEY_SIZE];
    if (!getKeyFromPassword(password, salt, key)) {
        fclose(inFile); fclose(outFile); remove(outputFile); return 0;
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { fclose(inFile); fclose(outFile); remove(outputFile); return 0; }

    int success = 1;
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) success = 0;
    if (success && 1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL)) success = 0;
    if (success && 1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) success = 0;

    unsigned char inBuffer[BUFFER_SIZE];
    unsigned char outBuffer[BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH];
    int len = 0;
    long totalRead = 0;
    int bytesToRead = 0;

    if (success) {
        while (totalRead < ciphertext_len) {
            bytesToRead = (ciphertext_len - totalRead > BUFFER_SIZE) ? BUFFER_SIZE : (ciphertext_len - totalRead);
            if (fread(inBuffer, 1, bytesToRead, inFile) != bytesToRead) {
                success = 0;
                break;
            }
            totalRead += bytesToRead;
            if (1 != EVP_DecryptUpdate(ctx, outBuffer, &len, inBuffer, bytesToRead)) {
                success = 0; // Likely wrong password or tampered
                break;
            }
            fwrite(outBuffer, 1, len, outFile);
        }
    }
    
    if (success && 1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, sizeof(tag), tag)) success = 0;

    if (success && 1 != EVP_DecryptFinal_ex(ctx, outBuffer, &len)) {
        // Authentication failed
        success = 0;
    }
    if (success) fwrite(outBuffer, 1, len, outFile);
    
    EVP_CIPHER_CTX_free(ctx);
    fclose(inFile);
    fclose(outFile);

    if (!success) { remove(outputFile); }
    return success;
}

int files_are_equal(const char* path1, const char* path2) {
    FILE* f1 = fopen(path1, "rb");
    FILE* f2 = fopen(path2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;
    }
    fseek(f1, 0, SEEK_END);
    fseek(f2, 0, SEEK_END);
    if (ftell(f1) != ftell(f2)) {
        fclose(f1); fclose(f2);
        return 0;
    }
    fseek(f1, 0, SEEK_SET);
    fseek(f2, 0, SEEK_SET);

    char buf1[BUFFER_SIZE], buf2[BUFFER_SIZE];
    size_t n1, n2;
    do {
        n1 = fread(buf1, 1, sizeof(buf1), f1);
        n2 = fread(buf2, 1, sizeof(buf2), f2);
        if (n1 != n2 || memcmp(buf1, buf2, n1) != 0) {
            fclose(f1); fclose(f2);
            return 0;
        }
    } while (n1 > 0);

    fclose(f1); fclose(f2);
    return 1;
}

void run_test(int num, const char* name, int condition) {
    printf("Test Case %d (%s): %s\n", num, name, (condition ? "PASSED" : "FAILED"));
}

int main() {
    printf("\nRunning C Tests...\n");
    const char* password = "my-very-secret-password-123";

    // Test Case 1: Simple text
    {
        FILE* f = fopen("plain1.txt", "w");
        fprintf(f, "This is a test file for encryption.");
        fclose(f);
        int success = encryptFile("plain1.txt", "encrypted1.enc", password) &&
                      decryptFile("encrypted1.enc", "decrypted1.txt", password);
        run_test(1, "Simple Text", success && files_are_equal("plain1.txt", "decrypted1.txt"));
        remove("plain1.txt"); remove("encrypted1.enc"); remove("decrypted1.txt");
    }
    
    // Test Case 2: Large binary
    {
        FILE* f = fopen("plain2.bin", "wb");
        char data[1024 * 100] = {0}; // 100KB
        fwrite(data, 1, sizeof(data), f);
        fclose(f);
        int success = encryptFile("plain2.bin", "encrypted2.enc", password) &&
                      decryptFile("encrypted2.enc", "decrypted2.bin", password);
        run_test(2, "Large File", success && files_are_equal("plain2.bin", "decrypted2.bin"));
        remove("plain2.bin"); remove("encrypted2.enc"); remove("decrypted2.bin");
    }

    // Test Case 3: Empty file
    {
        FILE* f = fopen("plain3.txt", "w"); fclose(f);
        int success = encryptFile("plain3.txt", "encrypted3.enc", password) &&
                      decryptFile("encrypted3.enc", "decrypted3.txt", password);
        run_test(3, "Empty File", success && files_are_equal("plain3.txt", "decrypted3.txt"));
        remove("plain3.txt"); remove("encrypted3.enc"); remove("decrypted3.txt");
    }
    
    // Test Case 4: Wrong password
    {
        FILE* f = fopen("plain4.txt", "w"); fprintf(f, "wrong password test"); fclose(f);
        int encrypted = encryptFile("plain4.txt", "encrypted4.enc", password);
        int decrypted = decryptFile("encrypted4.enc", "decrypted4.txt", "wrong-password");
        run_test(4, "Wrong Password", encrypted && !decrypted);
        remove("plain4.txt"); remove("encrypted4.enc"); remove("decrypted4.txt");
    }
    
    // Test Case 5: Tampered file
    {
        FILE* f = fopen("plain5.txt", "w"); fprintf(f, "tampered file test"); fclose(f);
        int encrypted = encryptFile("plain5.txt", "encrypted5.enc", password);
        
        // Tamper
        f = fopen("encrypted5.enc", "r+b");
        fseek(f, -5, SEEK_END);
        char byte;
        fread(&byte, 1, 1, f);
        fseek(f, -1, SEEK_CUR);
        byte ^= 1;
        fwrite(&byte, 1, 1, f);
        fclose(f);

        int decrypted = decryptFile("encrypted5.enc", "decrypted5.txt", password);
        run_test(5, "Tampered File", encrypted && !decrypted);
        remove("plain5.txt"); remove("encrypted5.enc"); remove("decrypted5.txt");
    }

    return 0;
}