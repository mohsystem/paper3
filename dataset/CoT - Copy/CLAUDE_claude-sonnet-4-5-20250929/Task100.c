
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_LINE_LENGTH 4096
#define MAX_MATCHES 100

void searchFileWithRegex(const char* regexPattern, const char* fileName) {
    // Validate inputs
    if (regexPattern == NULL || strlen(regexPattern) == 0) {
        fprintf(stderr, "Error: Regex pattern cannot be empty\\n");
        return;
    }
    
    if (fileName == NULL || strlen(fileName) == 0) {
        fprintf(stderr, "Error: File name cannot be empty\\n");
        return;
    }
    
    // Check file exists and get size
    struct stat st;
    if (stat(fileName, &st) != 0) {
        fprintf(stderr, "Error: File does not exist or cannot access: %s\\n", fileName);
        return;
    }
    
    // Check if regular file
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Path is not a regular file: %s\\n", fileName);
        return;
    }
    
    // Check file size
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File too large (max 10MB)\\n");
        return;
    }
    
    // Compile regex pattern
    regex_t regex;
    int reti = regcomp(&regex, regexPattern, REG_EXTENDED);
    if (reti != 0) {
        char error_buffer[256];
        regerror(reti, &regex, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "Error: Invalid regex pattern: %s\\n", error_buffer);
        return;
    }
    
    // Open file
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file: %s\\n", fileName);
        regfree(&regex);
        return;
    }
    
    // Read and search file content
    char line[MAX_LINE_LENGTH];
    int lineNumber = 0;
    int found = 0;
    regmatch_t matches[MAX_MATCHES];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        lineNumber++;
        
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\\n') {
            line[len - 1] = '\\0';
        }
        
        // Search for matches in line
        const char* cursor = line;
        size_t offset = 0;
        
        while (regexec(&regex, cursor, 1, matches, 0) == 0) {
            found = 1;
            
            printf("Line %d: %s\\n", lineNumber, line);
            
            // Calculate match position
            size_t matchStart = offset + matches[0].rm_so;
            size_t matchEnd = offset + matches[0].rm_eo;
            
            // Print match
            printf("  Match: \\"");
            for (size_t i = matchStart; i < matchEnd; i++) {
                putchar(line[i]);
            }
            printf("\\" at position %zu\\n", matchStart);
            
            // Move cursor forward
            offset += matches[0].rm_eo;
            cursor += matches[0].rm_eo;
            
            // Break if no more content
            if (*cursor == '\\0') break;
        }
    }
    
    if (!found) {
        printf("No matches found.\\n");
    }
    
    // Cleanup
    fclose(file);
    regfree(&regex);
}

void createTestFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "%s", content);
        fclose(file);
    }
}

void cleanupTestFiles() {
    remove("test1.txt");
    remove("test2.txt");
    remove("test3.txt");
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        searchFileWithRegex(argv[1], argv[2]);
    } else {
        // Test cases
        printf("=== Test Case 1: Search for email pattern ===\\n");
        createTestFile("test1.txt", "Contact: john@example.com\\nEmail: jane@test.org");
        searchFileWithRegex("[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}", "test1.txt");
        
        printf("\\n=== Test Case 2: Search for phone numbers ===\\n");
        createTestFile("test2.txt", "Call: 123-456-7890\\nPhone: 555-0123");
        searchFileWithRegex("[0-9]{3}-[0-9]{3}-[0-9]{4}", "test2.txt");
        
        printf("\\n=== Test Case 3: No matches ===\\n");
        createTestFile("test3.txt", "No numbers here at all!");
        searchFileWithRegex("[0-9]+", "test3.txt");
        
        printf("\\n=== Test Case 4: Invalid regex ===\\n");
        searchFileWithRegex("[invalid(", "test1.txt");
        
        printf("\\n=== Test Case 5: Non-existent file ===\\n");
        searchFileWithRegex("test", "nonexistent.txt");
        
        cleanupTestFiles();
    }
    
    return 0;
}
