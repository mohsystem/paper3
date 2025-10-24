#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Creates a temporary file, writes a random UTF-8 encoded string to it,
 * and returns the file path.
 * Note: tmpnam is generally considered unsafe due to race conditions.
 * More secure alternatives (e.g., mkstemp) are platform-specific.
 * @return A dynamically allocated string with the file path (must be freed by caller),
 *         or NULL on failure.
 */
char* createTempFileWithRandomUnicodeString() {
    // 1. Create a temporary file path.
    char* path_buffer = malloc(L_tmpnam);
    if (!path_buffer) {
        perror("Failed to allocate memory for path");
        return NULL;
    }
    if (tmpnam(path_buffer) == NULL) {
        perror("tmpnam failed");
        free(path_buffer);
        return NULL;
    }
    
    // 2. Create a random string.
    const char* chars[] = {
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
        "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
        "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "à", "á", "ç", "è", "é", "ê", "ë", "Α", "Β", "Γ", "Δ", "Ε",
        "€", "£", "¥", "©", "®", "™"
    };
    int num_chars = sizeof(chars) / sizeof(chars[0]);
    int string_len = 100;
    // Allocate a buffer. Max UTF-8 char is 4 bytes. 100 * 4 + 1 for null terminator.
    char* random_string = (char*)malloc(string_len * 4 + 1);
    if (!random_string) {
        perror("Failed to allocate memory for string");
        free(path_buffer);
        return NULL;
    }
    random_string[0] = '\0';

    for (int i = 0; i < string_len; ++i) {
        strcat(random_string, chars[rand() % num_chars]);
    }

    // 4. Write the string to the file in binary write mode ("wb").
    FILE* fp = fopen(path_buffer, "wb");
    if (fp == NULL) {
        perror("Failed to open temporary file");
        free(path_buffer);
        free(random_string);
        return NULL;
    }
    
    fputs(random_string, fp);
    fclose(fp);
    free(random_string);

    // 5. Return the path of the temporary file.
    return path_buffer;
}

int main() {
    srand((unsigned int)time(NULL));

    for (int i = 1; i <= 5; ++i) {
        printf("Test Case %d:\n", i);
        char* file_path = createTempFileWithRandomUnicodeString();
        if (file_path) {
            printf("  Temporary file created at: %s\n", file_path);

            // Cleanup the temporary file.
            if (remove(file_path) == 0) {
                printf("  Temporary file deleted.\n");
            } else {
                perror("  Error deleting temporary file");
            }
            
            // Free the memory allocated by the function for the path.
            free(file_path);
        } else {
            printf("  Failed to create temporary file.\n");
        }
        printf("\n");
    }
    return 0;
}