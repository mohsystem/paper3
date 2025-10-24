#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Helper function to reverse a string in-place.
 */
void reverse_string(char *str) {
    if (str == NULL) {
        return;
    }
    int len = strlen(str);
    int i = 0;
    int j = len - 1;
    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

/**
 * Reverses words in a string that have five or more letters.
 *
 * @param sentence The input string of one or more words.
 * @return A new dynamically allocated string with long words reversed.
 *         The caller is responsible for freeing this memory.
 */
char* spinWords(const char *sentence) {
    if (sentence == NULL) {
        return NULL;
    }

    // Duplicate the input string because strtok modifies it.
    char *input_copy = strdup(sentence);
    if (input_copy == NULL) {
        return NULL; // Failed to allocate memory
    }
    
    // Allocate memory for the result string.
    char *result = (char *)malloc(strlen(sentence) + 1);
    if (result == NULL) {
        free(input_copy);
        return NULL; // Failed to allocate memory
    }
    result[0] = '\0'; // Initialize as an empty string.

    const char *delim = " ";
    char *token = strtok(input_copy, delim);
    int is_first_word = 1;

    while (token != NULL) {
        if (!is_first_word) {
            strcat(result, " ");
        }

        if (strlen(token) >= 5) {
            reverse_string(token);
        }
        strcat(result, token);

        is_first_word = 0;
        token = strtok(NULL, delim);
    }

    free(input_copy);
    return result;
}

int main() {
    const char* testCases[] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "Just kidding there is still one more"
    };

    const char* expectedResults[] = {
        "Hey wollef sroirraw",
        "This is a test",
        "This is rehtona test",
        "emocleW",
        "Just gniddik ereht is llits one more"
    };
    
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        char *result = spinWords(testCases[i]);
        printf("Test Case %d:\n", i + 1);
        printf("Input:    \"%s\"\n", testCases[i]);
        printf("Output:   \"%s\"\n", result);
        printf("Expected: \"%s\"\n", expectedResults[i]);
        printf("Result: %s\n\n", strcmp(result, expectedResults[i]) == 0 ? "Passed" : "Failed");
        
        // The caller must free the memory allocated by spinWords
        free(result);
    }

    return 0;
}