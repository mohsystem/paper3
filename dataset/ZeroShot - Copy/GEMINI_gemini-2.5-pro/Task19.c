#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Reverses a substring in place given start and end pointers.
 * 
 * @param start Pointer to the first character of the substring.
 * @param end Pointer to the last character of the substring.
 */
void reverse_substring(char *start, char *end) {
    if (start == NULL || end == NULL) {
        return;
    }
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}

/**
 * @brief Reverses words in a sentence that have five or more letters.
 * 
 * @param sentence The input C-string.
 * @return A new dynamically allocated string with the modifications. 
 *         The caller is responsible for freeing this memory with free().
 *         Returns NULL on allocation failure.
 */
char* spinWords(const char* sentence) {
    if (sentence == NULL) {
        // strdup(NULL) is implementation-defined, better to handle it explicitly.
        char* empty_str = (char*)malloc(1);
        if (empty_str) empty_str[0] = '\0';
        return empty_str;
    }

    char* buffer = strdup(sentence);
    if (buffer == NULL) {
        // Failed to allocate memory
        return NULL;
    }

    char* word_start = buffer;
    char* current = buffer;

    while (*current) { // Loop until the null terminator
        if (*current == ' ') {
            // Found a word boundary. The word is from word_start to current-1.
            if ((current - word_start) >= 5) {
                reverse_substring(word_start, current - 1);
            }
            // The next word starts after this space
            word_start = current + 1;
        }
        current++;
    }

    // Handle the last word (or the only word if there are no spaces)
    if ((current - word_start) >= 5) {
        reverse_substring(word_start, current - 1);
    }

    return buffer;
}

int main() {
    const char* testCases[] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome to the jungle",
        "Supercalifragilisticexpialidocious"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        char* result = spinWords(testCases[i]);
        if (result != NULL) {
            printf("%s\n", result);
            free(result); // Free the memory allocated by spinWords
        }
    }

    return 0;
}