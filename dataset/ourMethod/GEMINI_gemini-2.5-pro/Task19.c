#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Reverses a substring in place.
 *
 * @param start Pointer to the first character of the substring.
 * @param end Pointer to the last character of the substring (inclusive).
 */
void reverse_substring(char* start, char* end) {
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
 * @brief Reverses words in a string that have five or more letters.
 *
 * This function takes a string, finds words separated by spaces,
 * and reverses any word that is five or more characters long. The
 * original spacing is preserved.
 *
 * @param sentence The input string. Must consist of only letters and spaces.
 * @return A new, dynamically allocated string with the specified words reversed.
 *         The caller is responsible for freeing this memory. Returns NULL on
 *         failure (e.g., memory allocation error, NULL input).
 */
char* spinWords(const char* sentence) {
    if (sentence == NULL) {
        return NULL;
    }

    size_t len = strlen(sentence);
    // Allocate memory for a mutable copy of the string
    char* result = (char*)malloc(len + 1);
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }
    // Safely copy the string, including the null terminator
    memcpy(result, sentence, len + 1);

    char* word_start = result;
    for (size_t i = 0; i <= len; ++i) {
        // A word ends at a space or at the end of the string
        if (i == len || result[i] == ' ') {
            // Check if the word length is 5 or more using pointer arithmetic
            if ((result + i) - word_start >= 5) {
                // Reverse the word in-place. The end pointer is the character before the space.
                reverse_substring(word_start, result + i - 1);
            }
            // Move to the start of the next potential word
            word_start = result + i + 1;
        }
    }
    return result;
}

int main() {
    const char* test_cases[] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "Just kidding there is still one more"
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        printf("Original: \"%s\"\n", test_cases[i]);
        char* spun_str = spinWords(test_cases[i]);
        if (spun_str != NULL) {
            printf("Spun:     \"%s\"\n\n", spun_str);
            free(spun_str); // Free the allocated memory
            spun_str = NULL; // Avoid using a dangling pointer
        } else {
            printf("Spun:     (Error occurred)\n\n");
        }
    }

    return 0;
}