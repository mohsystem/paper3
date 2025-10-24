#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
 * Searches a file for lines matching a given regular expression.
 *
 * @param regex_pattern The regular expression pattern to search for.
 * @param file_path     The path to the file to be searched.
 */
void search_file_with_regex(const char* regex_pattern, const char* file_path) {
    regex_t regex;
    int reti;
    char err_buf[100];

    // Security: Compile the regex and check for errors.
    reti = regcomp(&regex, regex_pattern, REG_EXTENDED);
    if (reti) {
        regerror(reti, &regex, err_buf, sizeof(err_buf));
        fprintf(stderr, "Error: Could not compile regex: %s\n", err_buf);
        return;
    }

    // Security: Open file in read-only mode.
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        perror("Error opening file");
        regfree(&regex); // Free the compiled regex before returning.
        return;
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_number = 0;

    // Security: getline handles memory allocation, preventing buffer overflows.
    while ((read = getline(&line, &len, fp)) != -1) {
        line_number++;
        // Remove trailing newline if it exists
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        
        // Execute the regex
        reti = regexec(&regex, line, 0, NULL, 0);
        if (!reti) {
            printf("%s:%d:%s\n", file_path, line_number, line);
        } else if (reti != REG_NOMATCH) {
            regerror(reti, &regex, err_buf, sizeof(err_buf));
            fprintf(stderr, "Regex match failed: %s\n", err_buf);
        }
    }

    // Security: Free all allocated resources.
    fclose(fp);
    if (line) {
        free(line);
    }
    regfree(&regex);
}

void run_tests() {
    const char* test_file_name = "test_file.txt";
    // Create a test file
    FILE* fp = fopen(test_file_name, "w");
    if (fp == NULL) {
        perror("Failed to create test file");
        return;
    }
    fprintf(fp, "Hello world, this is a test file.\n");
    fprintf(fp, "The year is 2024.\n");
    fprintf(fp, "Let's test some patterns.\n");
    fprintf(fp, "Another line for testing.\n");
    fprintf(fp, "bat, bit, but, b@t.\n");
    fprintf(fp, "Goodbye!\n");
    fclose(fp);

    printf("\n--- Test Case 1: Simple Match ('test') ---\n");
    search_file_with_regex("test", test_file_name);

    printf("\n--- Test Case 2: No Match ('nomatch') ---\n");
    search_file_with_regex("nomatch", test_file_name);

    printf("\n--- Test Case 3: Regex Special Chars ('b.t') ---\n");
    search_file_with_regex("b.t", test_file_name);

    printf("\n--- Test Case 4: Digit Match ('[0-9]+') ---\n");
    search_file_with_regex("[0-9]+", test_file_name);

    printf("\n--- Test Case 5: Non-existent File ---\n");
    search_file_with_regex("hello", "nonexistent.txt");

    // Clean up the test file
    remove(test_file_name);
}

int main(int argc, char* argv[]) {
    // Security: Check for the correct number of command-line arguments.
    if (argc == 3) {
        search_file_with_regex(argv[1], argv[2]);
    } else {
        printf("Usage: %s <regex_pattern> <file_path>\n", argv[0]);
        printf("Running test cases instead...\n");
        run_tests();
    }
    return 0;
}