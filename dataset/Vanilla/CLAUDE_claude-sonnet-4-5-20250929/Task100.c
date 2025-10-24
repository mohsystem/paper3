
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

void searchPatternInFile(const char* patternStr, const char* fileName) {
    regex_t regex;
    regmatch_t match;
    FILE* file;
    char* content = NULL;
    long fileSize;
    int reti;
    
    // Compile regex
    reti = regcomp(&regex, patternStr, REG_EXTENDED);
    if (reti) {
        printf("Invalid regex pattern\\n");
        return;
    }
    
    // Open and read file
    file = fopen(fileName, "r");
    if (!file) {
        printf("Error reading file: File not found - %s\\n", fileName);
        regfree(&regex);
        return;
    }
    
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    content = (char*)malloc(fileSize + 1);
    if (!content) {
        printf("Memory allocation error\\n");
        fclose(file);
        regfree(&regex);
        return;
    }
    
    fread(content, 1, fileSize, file);
    content[fileSize] = '\\0';
    fclose(file);
    
    printf("Searching for pattern: %s in file: %s\\n", patternStr, fileName);
    
    // Search for matches
    int count = 0;
    char* ptr = content;
    while (regexec(&regex, ptr, 1, &match, 0) == 0) {
        count++;
        int matchLen = match.rm_eo - match.rm_so;
        printf("Match %d: ", count);
        for (int i = 0; i < matchLen; i++) {
            printf("%c", ptr[match.rm_so + i]);
        }
        printf(" at position %ld\\n", (long)(ptr - content + match.rm_so));
        ptr += match.rm_eo;
    }
    
    if (count == 0) {
        printf("No matches found.\\n");
    } else {
        printf("Total matches: %d\\n", count);
    }
    
    free(content);
    regfree(&regex);
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        searchPatternInFile(argv[1], argv[2]);
    } else {
        printf("Running test cases...\\n\\n");
        
        // Create test files
        FILE* file;
        
        file = fopen("test1.txt", "w");
        fprintf(file, "Hello World! Hello Java!");
        fclose(file);
        
        file = fopen("test2.txt", "w");
        fprintf(file, "abc123def456ghi789");
        fclose(file);
        
        file = fopen("test3.txt", "w");
        fprintf(file, "email@example.com test@domain.org");
        fclose(file);
        
        file = fopen("test4.txt", "w");
        fprintf(file, "The quick brown fox jumps over the lazy dog");
        fclose(file);
        
        file = fopen("test5.txt", "w");
        fprintf(file, "Line1\\nLine2\\nLine3");
        fclose(file);
        
        // Test case 1
        printf("Test 1:\\n");
        searchPatternInFile("Hello", "test1.txt");
        printf("\\n");
        
        // Test case 2
        printf("Test 2:\\n");
        searchPatternInFile("[0-9]+", "test2.txt");
        printf("\\n");
        
        // Test case 3
        printf("Test 3:\\n");
        searchPatternInFile("[a-z]+@[a-z]+\\\\.[a-z]+", "test3.txt");
        printf("\\n");
        
        // Test case 4
        printf("Test 4:\\n");
        searchPatternInFile("\\\\<[a-zA-Z][a-zA-Z][a-zA-Z]\\\\>", "test4.txt");
        printf("\\n");
        
        // Test case 5
        printf("Test 5:\\n");
        searchPatternInFile("Line[0-9]", "test5.txt");
        printf("\\n");
    }
    
    return 0;
}
