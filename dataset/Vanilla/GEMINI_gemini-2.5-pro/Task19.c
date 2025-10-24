#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to reverse a string in-place
void reverse_string(char *str) {
    int length = strlen(str);
    for (int i = 0; i < length / 2; i++) {
        char temp = str[i];
        str[i] = str[length - i - 1];
        str[length - i - 1] = temp;
    }
}

char* spinWords(const char* sentence) {
    // Create a mutable copy of the input string for strtok
    char* buffer = (char*)malloc(strlen(sentence) + 1);
    if (buffer == NULL) return NULL;
    strcpy(buffer, sentence);

    // Allocate memory for the result string
    char* result = (char*)malloc(strlen(sentence) + 2); // +2 for null terminator and potential space
    if (result == NULL) {
        free(buffer);
        return NULL;
    }
    result[0] = '\0'; // Initialize as an empty string

    char* word = strtok(buffer, " ");
    int first_word = 1;

    while (word != NULL) {
        if (!first_word) {
            strcat(result, " "); // Add space before the next word
        }

        if (strlen(word) >= 5) {
            reverse_string(word);
        }
        strcat(result, word);

        first_word = 0;
        word = strtok(NULL, " ");
    }

    free(buffer); // Clean up the copied buffer
    return result; // Caller is responsible for freeing this memory
}

int main() {
    // Test Case 1
    const char* input1 = "Hey fellow warriors";
    char* output1 = spinWords(input1);
    printf("Input: \"%s\"\n", input1);
    printf("Output: \"%s\"\n\n", output1); // Expected: "Hey wollef sroirraw"
    free(output1);

    // Test Case 2
    const char* input2 = "This is a test";
    char* output2 = spinWords(input2);
    printf("Input: \"%s\"\n", input2);
    printf("Output: \"%s\"\n\n", output2); // Expected: "This is a test"
    free(output2);

    // Test Case 3
    const char* input3 = "This is another test";
    char* output3 = spinWords(input3);
    printf("Input: \"%s\"\n", input3);
    printf("Output: \"%s\"\n\n", output3); // Expected: "This is rehtona test"
    free(output3);

    // Test Case 4
    const char* input4 = "Welcome";
    char* output4 = spinWords(input4);
    printf("Input: \"%s\"\n", input4);
    printf("Output: \"%s\"\n\n", output4); // Expected: "emocleW"
    free(output4);

    // Test Case 5
    const char* input5 = "Just kidding there is still one more";
    char* output5 = spinWords(input5);
    printf("Input: \"%s\"\n", input5);
    printf("Output: \"%s\"\n\n", output5); // Expected: "Just gniddik ereht is llits one more"
    free(output5);

    return 0;
}