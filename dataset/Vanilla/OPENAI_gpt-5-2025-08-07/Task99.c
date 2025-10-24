#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

char* match_lines(const char* filename) {
    regex_t regex;
    // Pattern: start with one or more digits, anything, end with a dot
    const char* pattern = "^[0-9]+.*\\.$";
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        return strdup("");
    }

    FILE* f = fopen(filename, "rb");
    if (!f) {
        regfree(&regex);
        return strdup("");
    }

    char* result = NULL;
    size_t cap = 0;
    size_t len = 0;

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), f)) {
        // Strip trailing \r and 

        size_t L = strlen(buffer);
        while (L > 0 && (buffer[L - 1] == '\n' || buffer[L - 1] == '\r')) {
            buffer[--L] = '\0';
        }

        if (regexec(&regex, buffer, 0, NULL, 0) == 0) {
            size_t need = L + 1; // plus '\n'
            if (len + need + 1 > cap) {
                size_t newcap = (cap == 0) ? 1024 : cap * 2;
                while (len + need + 1 > newcap) newcap *= 2;
                char* tmp = (char*)realloc(result, newcap);
                if (!tmp) {
                    free(result);
                    fclose(f);
                    regfree(&regex);
                    return strdup("");
                }
                result = tmp;
                cap = newcap;
            }
            memcpy(result + len, buffer, L);
            len += L;
            result[len++] = '\n';
            result[len] = '\0';
        }
    }

    fclose(f);
    regfree(&regex);

    if (!result) return strdup("");
    // Optionally remove last trailing newline
    if (len > 0 && result[len - 1] == '\n') {
        result[len - 1] = '\0';
    }
    return result;
}

int main(void) {
    // Test case 1
    const char* f1 = "c_test1.txt";
    {
        FILE* f = fopen(f1, "wb");
        fprintf(f, "123 Start here.\n");
        fprintf(f, "Not starting with number.\n");
        fprintf(f, "42 Another line.\n");
        fprintf(f, "7 ends with no dot\n");
        fprintf(f, "0.\n");
        fclose(f);
    }
    char* r1 = match_lines(f1);
    printf("Test 1 matches:\n%s\n", r1);
    free(r1);

    // Test case 2
    const char* f2 = "c_test2.txt";
    {
        FILE* f = fopen(f2, "wb");
        fprintf(f, "10 Leading digits not ending with dot\n");
        fprintf(f, "20 ends with dot.\n");
        fprintf(f, ". starts with dot.\n");
        fprintf(f, "99Trailing spaces.\n");
        fclose(f);
    }
    char* r2 = match_lines(f2);
    printf("Test 2 matches:\n%s\n", r2);
    free(r2);

    // Test case 3
    const char* f3 = "c_test3.txt";
    {
        FILE* f = fopen(f3, "wb");
        fprintf(f, "No numbers at start.\n");
        fprintf(f, "3.14159.\n");
        fprintf(f, "31415\n");
        fprintf(f, "2718.\n");
        fclose(f);
    }
    char* r3 = match_lines(f3);
    printf("Test 3 matches:\n%s\n", r3);
    free(r3);

    // Test case 4
    const char* f4 = "c_test4.txt";
    {
        FILE* f = fopen(f4, "wb");
        fprintf(f, "A line.\n");
        fprintf(f, "123.\n");
        fprintf(f, "abc123.\n");
        fprintf(f, "456 trailing spaces .\n");
        fprintf(f, "789!.\n");
        fclose(f);
    }
    char* r4 = match_lines(f4);
    printf("Test 4 matches:\n%s\n", r4);
    free(r4);

    // Test case 5 (CRLF endings)
    const char* f5 = "c_test5.txt";
    {
        FILE* f = fopen(f5, "wb");
        fputs("100 First line.\r\n", f);
        fputs("no number.\r\n", f);
        fputs("200 End with dot.\r\n", f);
        fputs("300 no dot\r\n", f);
        fputs("400.\r\n", f);
        fclose(f);
    }
    char* r5 = match_lines(f5);
    printf("Test 5 matches:\n%s\n", r5);
    free(r5);

    return 0;
}