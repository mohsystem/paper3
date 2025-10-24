
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char* accum(const char* s) {
    if (s == NULL || strlen(s) == 0) {
        char* empty = (char*)malloc(1);
        empty[0] = '\\0';
        return empty;
    }
    
    int len = strlen(s);
    int result_size = 0;
    for (int i = 0; i < len; i++) {
        result_size += (i + 1) + 1;
    }
    
    char* result = (char*)malloc(result_size);
    int pos = 0;
    
    for (int i = 0; i < len; i++) {
        result[pos++] = toupper(s[i]);
        for (int j = 0; j < i; j++) {
            result[pos++] = tolower(s[i]);
        }
        if (i < len - 1) {
            result[pos++] = '-';
        }
    }
    result[pos] = '\\0';
    
    return result;
}

int main() {
    char* result1 = accum("abcd");
    printf("%s\\n", result1);
    free(result1);
    
    char* result2 = accum("RqaEzty");
    printf("%s\\n", result2);
    free(result2);
    
    char* result3 = accum("cwAt");
    printf("%s\\n", result3);
    free(result3);
    
    char* result4 = accum("ZpglnRxqenU");
    printf("%s\\n", result4);
    free(result4);
    
    char* result5 = accum("NyffsGeyylB");
    printf("%s\\n", result5);
    free(result5);
    
    return 0;
}
