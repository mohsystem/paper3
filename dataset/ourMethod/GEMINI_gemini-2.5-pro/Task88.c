#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For mkstemp, unlink on POSIX
#include <errno.h>
#include <openssl/rand.h>

// This code requires linking with OpenSSL's crypto library.
// Example compilation: gcc your_source.c -o program -lssl -lcrypto
//
// NOTE: This implementation uses mkstemp, which is a POSIX function.
// For Windows, you would use GetTempPathA and GetTempFileNameA to create a unique file name,
// then CreateFileA with CREATE_NEW to securely create it.

/**
 * @brief Creates a temporary file, writes a random string's Unicode representation to it.
 *
 * @return A dynamically allocated string containing the file path, or NULL on failure.
 *         The caller is responsible for freeing this string.
 */
char* createTempFileWithUnicodeString() {
    // 1. Create a temporary file securely using mkstemp.
    char template_path[] = "/tmp/task88_XXXXXX"; // Template for mkstemp
    int fd = mkstemp(template_path);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }

    // Associate a stream with the file descriptor.
    FILE* temp_file = fdopen(fd, "w");
    if (temp_file == NULL) {
        perror("fdopen failed");
        close(fd);
        unlink(template_path); // Clean up the created file on error
        return NULL;
    }

    // 2. Create a random string using OpenSSL's CSPRNG.
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const int charset_size = sizeof(charset) - 1;
    const int string_length = 32;
    unsigned char rand_bytes[string_length];

    if (RAND_bytes(rand_bytes, string_length) != 1) {
        fprintf(stderr, "RAND_bytes failed\n");
        fclose(temp_file); // This also closes fd
        unlink(template_path);
        return NULL;
    }

    // 3. Convert string to Unicode representation and write to the file.
    for (int i = 0; i < string_length; ++i) {
        char random_char = charset[rand_bytes[i] % charset_size];
        if (fprintf(temp_file, "U+%04X ", (unsigned char)random_char) < 0) {
            perror("fprintf failed");
            fclose(temp_file);
            unlink(template_path);
            return NULL;
        }
    }

    if (fclose(temp_file) != 0) {
        perror("fclose failed");
        unlink(template_path);
        return NULL;
    }

    // 4. Return the path of the temporary file.
    char* result_path = malloc(strlen(template_path) + 1);
    if (result_path == NULL) {
        perror("malloc failed");
        unlink(template_path);
        return NULL;
    }
    // strcpy is safe here because the destination buffer was allocated to be the exact size of the source.
    strcpy(result_path, template_path); 
    
    return result_path;
}

int main() {
    for (int i = 0; i < 5; ++i) {
        char* file_path = NULL;
        file_path = createTempFileWithUnicodeString();
        
        if (file_path != NULL) {
            printf("Test Case %d: Created temp file at: %s\n", i + 1, file_path);
            
            // Optional: Read and print content for verification
            FILE* f = fopen(file_path, "r");
            if (f) {
                char buffer[256];
                printf("  File content: ");
                if (fgets(buffer, sizeof(buffer), f) != NULL) {
                    printf("%s\n", buffer);
                }
                fclose(f);
            } else {
                perror("  Failed to open file for reading");
            }
            
            // Clean up the file and allocated memory
            if (unlink(file_path) == 0) {
                 printf("  Cleaned up temp file: %s\n", file_path);
            } else {
                 perror("  Failed to clean up temp file");
            }
            free(file_path);
            file_path = NULL;
        } else {
            fprintf(stderr, "Test Case %d: Failed to create temporary file.\n", i + 1);
        }
    }
    return 0;
}