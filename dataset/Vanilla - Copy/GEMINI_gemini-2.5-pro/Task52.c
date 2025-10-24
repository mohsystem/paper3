#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Encrypts or decrypts a file using a simple XOR cipher with a given key.
 * The operation is symmetric; applying it twice restores the original file.
 *
 * @param key The secret key for encryption/decryption.
 * @param inputFilePath Path to the input file.
 * @param outputFilePath Path to the output file.
 */
void processFile(const char* key, const char* inputFilePath, const char* outputFilePath) {
    if (key == NULL || strlen(key) == 0) {
        fprintf(stderr, "Error: Encryption key cannot be empty.\n");
        return;
    }

    FILE *inputFile = fopen(inputFilePath, "rb");
    if (inputFile == NULL) {
        perror("Error opening input file");
        return;
    }

    FILE *outputFile = fopen(outputFilePath, "wb");
    if (outputFile == NULL) {
        perror("Error opening output file");
        fclose(inputFile);
        return;
    }

    int c;
    size_t keyIndex = 0;
    size_t keyLen = strlen(key);

    // Read one byte at a time from the input file
    while ((c = fgetc(inputFile)) != EOF) {
        // XOR the byte with the corresponding key byte and write to the output file
        fputc(c ^ key[keyIndex], outputFile);
        // Move to the next key byte, wrapping around if necessary
        keyIndex = (keyIndex + 1) % keyLen;
    }

    fclose(inputFile);
    fclose(outputFile);
}

// Helper function to run a complete test case
void runTestCase(const char* testId, const char* content, const char* key) {
    char plainFile[100];
    char encryptedFile[100];
    char decryptedFile[100];

    sprintf(plainFile, "%s_plain.txt", testId);
    sprintf(encryptedFile, "%s_encrypted.bin", testId);
    sprintf(decryptedFile, "%s_decrypted.txt", testId);

    // 1. Create the original file
    FILE* originalFile = fopen(plainFile, "w");
    if (originalFile) {
        fputs(content, originalFile);
        fclose(originalFile);
    }
    printf("\n--- Running Test Case: %s ---\n", testId);
    printf("Original content: \"%s\"\n", content);

    // 2. Encrypt
    processFile(key, plainFile, encryptedFile);
    printf("'%s' encrypted to '%s' with key '%s'.\n", plainFile, encryptedFile, key);

    // 3. Decrypt
    processFile(key, encryptedFile, decryptedFile);
    printf("'%s' decrypted to '%s'.\n", encryptedFile, decryptedFile);

    // 4. Verify
    FILE* file = fopen(decryptedFile, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char* buffer = (char*)malloc(size + 1);
        if (buffer) {
            fread(buffer, 1, size, file);
            buffer[size] = '\0';

            if (strcmp(content, buffer) == 0) {
                printf("SUCCESS: Decrypted content matches original content.\n");
            } else {
                printf("FAILURE: Content mismatch!\n");
                printf("Decrypted content: \"%s\"\n", buffer);
            }
            free(buffer);
        }
        fclose(file);
    } else {
        printf("FAILURE: Could not open decrypted file for verification.\n");
    }

    // 5. Cleanup
    remove(plainFile);
    remove(encryptedFile);
    remove(decryptedFile);
}

int main() {
    printf("--- C File Encryption/Decryption Tests ---\n");

    // Test Case 1: Simple text
    runTestCase("test1", "Hello World!", "KEY1");
    
    // Test Case 2: Different key and text
    runTestCase("test2", "C XOR Encryption/Decryption.", "SECRET");
    
    // Test Case 3: Longer text to test key wrapping
    runTestCase("test3", "This is a longer piece of text to ensure the key repeats correctly.", "LONGERKEY");

    // Test Case 4: Text with special characters and numbers
    runTestCase("test4", "12345!@#$%^&*()_+", "NUMSYM");

    // Test Case 5: Empty file
    runTestCase("test5", "", "EMPTY");
    
    printf("\n--- C Tests Completed ---\n");
    return 0;
}