
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char* accum(const char* s) {
    int len = strlen(s);
    int resultLen = 0;
    for (int i = 0; i < len; i++) {
        resultLen += i + 1 + 1;
    }
    resultLen--;
    
    char* result = (char*)malloc((resultLen + 1) * sizeof(char));
    int pos = 0;
    
    for (int i = 0; i < len; i++) {
        char c = s[i];
        result[pos++] = toupper(c);
        for (int j = 0; j < i; j++) {
            result[pos++] = tolower(c);
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
