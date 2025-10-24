/*
 * NOTE: This code requires the OpenSSL library.
 * To compile on Linux/macOS:
 * gcc your_file_name.c -o your_program_name -lssl -lcrypto
 *
 * On Windows, you will need to install OpenSSL and link the libraries
 * (libssl.lib, libcrypto.lib) in your compiler settings.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define AES_256_GCM_KEY_SIZE 32
#define AES_256_GCM_IV_SIZE  12
#define AES_256_GCM_TAG_SIZE 16
#define BUFFER_SIZE 4096

void handle_openssl_errors() {
    ERR_print_errors_fp(stderr);
    // In a real application, you might not want to abort immediately
    // but for this example, it's clear.
}

int encrypt_decrypt_file(const char* inputFile, const char* outputFile, const unsigned char* key, int do_encrypt) {
    FILE* f_in = fopen(inputFile, "rb");
    if (!f_in) {
        perror("Failed to open input file");
        return 0;
    }

    FILE* f_out = fopen(outputFile, "wb");
    if (!f_out) {
        perror("Failed to open output file");
        fclose(f_in);
        return 0;
    }

    unsigned char iv[AES_256_GCM_IV_SIZE];
    unsigned char tag[AES_256_GCM_TAG_SIZE];
    
    if (do_encrypt) {
        if (RAND_bytes(iv, sizeof(iv)) != 1) {
            handle_openssl_errors();
            fclose(f_in);
            fclose(f_out);
            return 0;
        }
        // Prepend IV to the output file
        if (fwrite(iv, 1, sizeof(iv), f_out) != sizeof(iv)) {
            perror("Failed to write IV to output file");
            fclose(f_in);
            fclose(f_out);
            return 0;
        }
    } else { // Decrypt
        // Read IV from the start of the input file
        if (fread(iv, 1, sizeof(iv), f_in) != sizeof(iv)) {
            fprintf(stderr, "Failed to read IV from input file.\n");
            fclose(f_in);
            fclose(f_out);
            return 0;
        }
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        handle_openssl_errors();
        fclose(f_in);
        fclose(f_out);
        return 0;
    }

    if (EVP_CipherInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv, do_encrypt) != 1) {
        handle_openssl_errors();
        goto cleanup;
    }

    unsigned char in_buf[BUFFER_SIZE];
    unsigned char out_buf[BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH];
    int num_bytes_read, out_len;
    int success = 1;

    // Process file in chunks
    while ((num_bytes_read = fread(in_buf, 1, BUFFER_SIZE, f_in)) > 0) {
        // For decryption, the last chunk containing the tag is handled separately
        if (!do_encrypt && num_bytes_read <= AES_256_GCM_TAG_SIZE && feof(f_in)) {
             memcpy(tag, in_buf + num_bytes_read - AES_256_GCM_TAG_SIZE, AES_256_GCM_TAG_SIZE);
             num_bytes_read -= AES_256_GCM_TAG_SIZE;
             if (num_bytes_read < 0) num_bytes_read = 0; // handle empty file case
        }

        if (num_bytes_read > 0) {
             if (EVP_CipherUpdate(ctx, out_buf, &out_len, in_buf, num_bytes_read) != 1) {
                handle_openssl_errors();
                success = 0;
                goto cleanup;
            }
            if (fwrite(out_buf, 1, out_len, f_out) != out_len) {
                perror("Failed to write to output file");
                success = 0;
                goto cleanup;
            }
        }
    }

    if (do_encrypt) {
        if (EVP_CipherFinal_ex(ctx, out_buf, &out_len) != 1) {
            handle_openssl_errors();
            success = 0;
            goto cleanup;
        }
        if (fwrite(out_buf, 1, out_len, f_out) != out_len) {
            perror("Failed to write final block to output file");
            success = 0;
            goto cleanup;
        }
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, AES_256_GCM_TAG_SIZE, tag) != 1) {
            handle_openssl_errors();
            success = 0;
            goto cleanup;
        }
        if (fwrite(tag, 1, sizeof(tag), f_out) != sizeof(tag)) {
            perror("Failed to write tag to output file");
            success = 0;
            goto cleanup;
        }
    } else { // Decrypt
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, AES_256_GCM_TAG_SIZE, tag) != 1) {
            fprintf(stderr, "Decryption failed: could not set tag. File might be too short.\n");
            success = 0;
            goto cleanup;
        }
        if (EVP_CipherFinal_ex(ctx, out_buf, &out_len) != 1) {
            fprintf(stderr, "Decryption failed: Incorrect key or tampered data (tag mismatch).\n");
            success = 0;
            goto cleanup;
        }
        if (fwrite(out_buf, 1, out_len, f_out) != out_len) {
            perror("Failed to write final block to output file");
            success = 0;
            goto cleanup;
        }
    }
    
cleanup:
    EVP_CIPHER_CTX_free(ctx);
    fclose(f_in);
    fclose(f_out);
    return success;
}

int main(void) {
    const char* ORIGINAL_FILE = "original.txt";
    const char* ENCRYPTED_FILE = "encrypted.bin";
    const char* DECRYPTED_FILE = "decrypted.txt";
    const char* TAMPERED_FILE = "tampered.bin";
    const char* EMPTY_FILE = "empty.txt";
    const char* EMPTY_ENC_FILE = "empty.enc";
    const char* EMPTY_DEC_FILE = "empty.dec";
    const char* NON_EXISTENT_FILE = "non_existent.txt";

    unsigned char correctKey[AES_256_GCM_KEY_SIZE];
    unsigned char wrongKey[AES_256_GCM_KEY_SIZE];
    RAND_bytes(correctKey, sizeof(correctKey));
    RAND_bytes(wrongKey, sizeof(wrongKey));

    const char* originalContent = "This is a secret message for testing AES-GCM file encryption.";
    
    // --- Test Case 1: Successful Encryption and Decryption ---
    printf("--- Test Case 1: Successful Encryption/Decryption ---\n");
    FILE* f = fopen(ORIGINAL_FILE, "w");
    fputs(originalContent, f);
    fclose(f);
    
    int encryptSuccess = encrypt_decrypt_file(ORIGINAL_FILE, ENCRYPTED_FILE, correctKey, 1);
    int decryptSuccess = encrypt_decrypt_file(ENCRYPTED_FILE, DECRYPTED_FILE, correctKey, 0);

    f = fopen(DECRYPTED_FILE, "r");
    char buffer[256];
    fgets(buffer, sizeof(buffer), f);
    fclose(f);
    if (encryptSuccess && decryptSuccess && strcmp(originalContent, buffer) == 0) {
        printf("Test 1 Passed: Content matches after decryption.\n");
    } else {
        printf("Test 1 Failed.\n");
    }
    printf("--------------------------------------------------\n\n");

    // --- Test Case 2: Decryption with Wrong Key ---
    printf("--- Test Case 2: Decryption with Wrong Key ---\n");
    int decryptWrongKey = encrypt_decrypt_file(ENCRYPTED_FILE, "decrypted_wrong.txt", wrongKey, 0);
    if (!decryptWrongKey) {
        printf("Test 2 Passed: Decryption failed as expected.\n");
    } else {
        printf("Test 2 Failed: Decryption succeeded with a wrong key.\n");
    }
    printf("--------------------------------------------------\n\n");

    // --- Test Case 3: Decryption of Tampered File ---
    printf("--- Test Case 3: Decryption of Tampered File ---\n");
    FILE *f_in = fopen(ENCRYPTED_FILE, "rb");
    FILE *f_out = fopen(TAMPERED_FILE, "wb");
    char c;
    while((c = fgetc(f_in)) != EOF) fputc(c, f_out);
    fclose(f_in);
    fclose(f_out);
    // Tamper the file
    f = fopen(TAMPERED_FILE, "r+b");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size > AES_256_GCM_IV_SIZE + 1) {
        fseek(f, -2, SEEK_END); // Go to second to last byte
        char byte;
        fread(&byte, 1, 1, f);
        byte ^= 1; // Flip a bit
        fseek(f, -2, SEEK_END);
        fwrite(&byte, 1, 1, f);
    }
    fclose(f);
    
    int decryptTampered = encrypt_decrypt_file(TAMPERED_FILE, "decrypted_tampered.txt", correctKey, 0);
    if (!decryptTampered) {
        printf("Test 3 Passed: Decryption of tampered file failed as expected.\n");
    } else {
        printf("Test 3 Failed: Decryption of tampered file succeeded.\n");
    }
    printf("--------------------------------------------------\n\n");

    // --- Test Case 4: Encrypting and Decrypting an Empty File ---
    printf("--- Test Case 4: Encrypt/Decrypt Empty File ---\n");
    f = fopen(EMPTY_FILE, "w"); fclose(f); // Create empty file
    int encryptEmpty = encrypt_decrypt_file(EMPTY_FILE, EMPTY_ENC_FILE, correctKey, 1);
    int decryptEmpty = encrypt_decrypt_file(EMPTY_ENC_FILE, EMPTY_DEC_FILE, correctKey, 0);
    f = fopen(EMPTY_DEC_FILE, "r");
    fseek(f, 0, SEEK_END);
    long empty_size = ftell(f);
    fclose(f);
    if (encryptEmpty && decryptEmpty && empty_size == 0) {
        printf("Test 4 Passed: Empty file handled correctly.\n");
    } else {
        printf("Test 4 Failed.\n");
    }
    printf("--------------------------------------------------\n\n");

    // --- Test Case 5: Handling Non-Existent Input File ---
    printf("--- Test Case 5: Handling Non-Existent Input File ---\n");
    int encryptNonExistent = encrypt_decrypt_file(NON_EXISTENT_FILE, "out.enc", correctKey, 1);
    if (!encryptNonExistent) {
        printf("Test 5 Passed: Handled non-existent input file correctly.\n");
    } else {
        printf("Test 5 Failed.\n");
    }
    printf("--------------------------------------------------\n\n");
    
    // --- Cleanup ---
    remove(ORIGINAL_FILE);
    remove(ENCRYPTED_FILE);
    remove(DECRYPTED_FILE);
    remove("decrypted_wrong.txt");
    remove(TAMPERED_FILE);
    remove("decrypted_tampered.txt");
    remove(EMPTY_FILE);
    remove(EMPTY_ENC_FILE);
    remove(EMPTY_DEC_FILE);

    return 0;
}