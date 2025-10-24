#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Transposes the given content where rows are separated by newlines
 * and columns by spaces. The caller is responsible for freeing the returned string.
 *
 * @param content The string content to transpose.
 * @return A new dynamically allocated string with the transposed content,
 *         or NULL on failure.
 */
char* transposeContent(const char* content) {
    if (content == NULL || *content == '\0') {
        char* empty_result = (char*)malloc(1);
        if (empty_result) empty_result[0] = '\0';
        return empty_result;
    }

    // --- 1. Count rows and columns ---
    int numRows = 0;
    int numCols = 0;
    
    char* temp_str_rows = strdup(content);
    if (!temp_str_rows) return NULL;
    char* line = strtok(temp_str_rows, "\n");
    while (line) {
        numRows++;
        line = strtok(NULL, "\n");
    }
    free(temp_str_rows);

    if (numRows == 0) {
        char* empty_result = (char*)malloc(1);
        if (empty_result) empty_result[0] = '\0';
        return empty_result;
    }

    char* temp_str_cols = strdup(content);
    if (!temp_str_cols) return NULL;
    line = strtok(temp_str_cols, "\n");
    if (line) {
        char* word = strtok(line, " ");
        while (word) {
            numCols++;
            word = strtok(NULL, " ");
        }
    }
    free(temp_str_cols);

    if (numCols == 0) {
        char* empty_result = (char*)malloc(1);
        if (empty_result) empty_result[0] = '\0';
        return empty_result;
    }

    // --- 2. Parse and Store pointers to words ---
    char* content_copy = strdup(content);
    if (!content_copy) return NULL;

    char** words = (char**)malloc(numRows * numCols * sizeof(char*));
    if (!words) {
        free(content_copy);
        return NULL;
    }

    int word_idx = 0;
    char* line_saveptr;
    char* current_line = strtok_r(content_copy, "\n", &line_saveptr);
    while (current_line) {
        char* word_saveptr;
        char* word = strtok_r(current_line, " ", &word_saveptr);
        while (word) {
            if (word_idx < numRows * numCols) {
                words[word_idx++] = word;
            }
            word = strtok_r(NULL, " ", &word_saveptr);
        }
        current_line = strtok_r(NULL, "\n", &line_saveptr);
    }

    // --- 3. Build output string ---
    size_t output_len = strlen(content) + 1;
    char* result = (char*)malloc(output_len);
    if (!result) {
        free(words);
        free(content_copy);
        return NULL;
    }
    
    char* result_p = result;
    for (int j = 0; j < numCols; ++j) {
        for (int i = 0; i < numRows; ++i) {
            char* word = words[i * numCols + j];
            int len = strlen(word);
            memcpy(result_p, word, len);
            result_p += len;

            if (i < numRows - 1) {
                *result_p++ = ' ';
            }
        }
        if (j < numCols - 1) {
            *result_p++ = '\n';
        }
    }
    *result_p = '\0';

    // --- 4. Cleanup ---
    free(words);
    free(content_copy);
    
    return result;
}

int main() {
    const char* testCases[] = {
        "name age\nalice 21\nryan 30",
        "a b c\nd e f\ng h i",
        "one two three four",
        "apple\nbanana\ncherry",
        ""
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("Test Case %d:\n", i + 1);
        printf("Input:\n");
        printf("%s\n", strlen(testCases[i]) == 0 ? "<empty>" : testCases[i]);
        char* result = transposeContent(testCases[i]);
        printf("Output:\n");
        if (result) {
            printf("%s\n", strlen(result) == 0 ? "<empty>" : result);
            free(result);
        } else {
            printf("<null>\n");
        }
        printf("--------------------\n");
    }

    return 0;
}