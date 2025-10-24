#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>

#define LINE_BUFFER_SIZE 4096
#define REGEX_ERROR_BUFFER_SIZE 256

/**
 * Searches a file for lines matching a regular expression.
 *
 * @param pattern_str The regular expression to search for.
 * @param file_path   The path to the file to search in.
 */
void process_file(const char* pattern_str, const char* file_path) {
    struct stat st;
    if (stat(file_path, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: File not found or is not a regular file: %s\n", file_path);
        return;
    }

    FILE* f = fopen(file_path, "r");
    if (f == NULL) {
        perror("Error opening file");
        return;
    }

    regex_t regex;
    int reti = regcomp(&regex, pattern_str, REG_EXTENDED);
    if (reti) {
        char msgbuf[REGEX_ERROR_BUFFER_SIZE];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex compilation failed: %s\n", msgbuf);
        fclose(f);
        return;
    }

    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), f) != NULL) {
        reti = regexec(&regex, line, 0, NULL, 0);
        if (reti == 0) { // Match found
            printf("%s", line);
        } else if (reti != REG_NOMATCH) { // An error occurred
            char msgbuf[REGEX_ERROR_BUFFER_SIZE];
            regerror(reti, &regex, msgbuf, sizeof(msgbuf));
            fprintf(stderr, "Regex match failed: %s\n", msgbuf);
            break; 
        }
    }

    regfree(&regex);
    fclose(f);
}

/**
 * Runs a series of test cases.
 */
void run_tests() {
    printf("Running built-in tests...\n");
    const char* test_filename = "test_regex_search_c.txt";
    
    FILE* f = fopen(test_filename, "w");
    if (f == NULL) {
        perror("Failed to create test file");
        return;
    }
    fprintf(f, "Hello World\n");
    fprintf(f, "This is a test file.\n");
    fprintf(f, "The quick brown fox jumps over the lazy dog.\n");
    fprintf(f, "Contact us at test@example.com or support@example.org.\n");
    fprintf(f, "Phone numbers: 123-456-7890, (987)654-3210.\n");
    fprintf(f, "Another line with numbers 12345.\n");
    fprintf(f, "end of file.\n");
    fclose(f);

    printf("\n--- Test Case 1: Find lines with 'fox' ---\n");
    process_file("fox", test_filename);

    printf("\n--- Test Case 2: Find lines with any number ---\n");
    process_file("[0-9]+", test_filename);

    printf("\n--- Test Case 3: Find lines with email addresses ---\n");
    process_file("[a-zA-Z0-9._%%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}", test_filename);

    printf("\n--- Test Case 4: Find lines starting with 'The' ---\n");
    process_file("^The", test_filename);

    printf("\n--- Test Case 5: Find lines with 'nonexistentpattern' ---\n");
    process_file("nonexistentpattern", test_filename);
    printf("(No output expected)\n");

    remove(test_filename);
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        process_file(argv[1], argv[2]);
    } else {
        printf("Usage: %s \"<regex_pattern>\" \"<file_name>\"\n", argv[0]);
        printf("No command-line arguments provided, running test cases.\n");
        run_tests();
    }
    return 0;
}