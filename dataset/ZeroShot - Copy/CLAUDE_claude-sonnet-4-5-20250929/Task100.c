
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>

#define MAX_MATCHES 1000
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_BUFFER 10000

char* searchPatternInFile(const char* pattern, const char* filename) {
    static char result[MAX_BUFFER];
    result[0] = '\\0';
    
    regex_t regex;
    regmatch_t matches[1];
    FILE* file;
    struct stat st;
    char* content = NULL;
    
    // Validate input parameters
    if (pattern == NULL || strlen(pattern) == 0) {
        snprintf(result, MAX_BUFFER, "Error: Pattern cannot be empty");
        return result;
    }
    if (filename == NULL || strlen(filename) == 0) {
        snprintf(result, MAX_BUFFER, "Error: Filename cannot be empty");
        return result;
    }
    
    // Check if file exists
    if (stat(filename, &st) != 0) {
        snprintf(result, MAX_BUFFER, "Error: File does not exist");
        return result;
    }
    
    // Check file size
    if (st.st_size > MAX_FILE_SIZE) {
        snprintf(result, MAX_BUFFER, "Error: File too large (max 10MB)");
        return result;
    }
    
    // Compile regex
    int reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti != 0) {
        snprintf(result, MAX_BUFFER, "Error: Invalid regex pattern");
        return result;
    }
    
    // Open and read file
    file = fopen(filename, "r");
    if (file == NULL) {
        regfree(&regex);
        snprintf(result, MAX_BUFFER, "Error: Cannot open file");
        return result;
    }
    
    content = (char*)malloc(st.st_size + 1);
    if (content == NULL) {
        fclose(file);
        regfree(&regex);
        snprintf(result, MAX_BUFFER, "Error: Memory allocation failed");
        return result;
    }
    
    size_t bytes_read = fread(content, 1, st.st_size, file);
    content[bytes_read] = '\\0';
    fclose(file);
    
    // Search for matches
    int matchCount = 0;
    char* p = content;
    char temp[256];
    
    while (regexec(&regex, p, 1, matches, 0) == 0 && matchCount < MAX_MATCHES) {
        int start = matches[0].rm_so;
        int end = matches[0].rm_eo;
        int match_len = end - start;
        
        snprintf(temp, sizeof(temp), "Match found at position %ld: %.*s\\n", 
                 (long)(p - content + start), match_len, p + start);
        strncat(result, temp, MAX_BUFFER - strlen(result) - 1);
        
        p += end;
        matchCount++;
    }
    
    if (matchCount == 0) {
        strncat(result, "No matches found\\n", MAX_BUFFER - strlen(result) - 1);
    } else {
        snprintf(temp, sizeof(temp), "Total matches: %d\\n", matchCount);
        strncat(result, temp, MAX_BUFFER - strlen(result) - 1);
    }
    
    free(content);
    regfree(&regex);
    
    return result;
}

void createTestFile(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "%s", content);
        fclose(file);
    }
}

void deleteTestFile(const char* filename) {
    remove(filename);
}

int main() {
    printf("=== Test Case 1: Search for word pattern ===\\n");
    createTestFile("test1.txt", "Hello world! Hello Java programming.");
    printf("%s\\n", searchPatternInFile("Hello", "test1.txt"));
    
    printf("=== Test Case 2: Search for email pattern ===\\n");
    createTestFile("test2.txt", "Contact: user@example.com or admin@test.org");
    printf("%s\\n", searchPatternInFile("[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}", "test2.txt"));
    
    printf("=== Test Case 3: Search for digit pattern ===\\n");
    createTestFile("test3.txt", "The numbers are 123, 456, and 789.");
    printf("%s\\n", searchPatternInFile("[0-9]+", "test3.txt"));
    
    printf("=== Test Case 4: Invalid pattern ===\\n");
    printf("%s\\n", searchPatternInFile("[invalid(", "test1.txt"));
    
    printf("=== Test Case 5: Non-existent file ===\\n");
    printf("%s\\n", searchPatternInFile("test", "nonexistent.txt"));
    
    // Cleanup
    deleteTestFile("test1.txt");
    deleteTestFile("test2.txt");
    deleteTestFile("test3.txt");
    
    return 0;
}
