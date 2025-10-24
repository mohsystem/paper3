
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* expandedForm(int num) {
    char numStr[20];
    sprintf(numStr, "%d", num);
    int length = strlen(numStr);
    
    char* result = (char*)malloc(1000 * sizeof(char));
    result[0] = '\\0';
    
    for (int i = 0; i < length; i++) {
        char digit = numStr[i];
        if (digit != '0') {
            if (strlen(result) > 0) {
                strcat(result, " + ");
            }
            char temp[20];
            temp[0] = digit;
            temp[1] = '\\0';
            for (int j = 0; j < length - i - 1; j++) {
                strcat(temp, "0");
            }
            strcat(result, temp);
        }
    }
    
    return result;
}

int main() {
    char* result1 = expandedForm(12);
    printf("%s\\n", result1);
    free(result1);
    
    char* result2 = expandedForm(42);
    printf("%s\\n", result2);
    free(result2);
    
    char* result3 = expandedForm(70304);
    printf("%s\\n", result3);
    free(result3);
    
    char* result4 = expandedForm(9000);
    printf("%s\\n", result4);
    free(result4);
    
    char* result5 = expandedForm(123456);
    printf("%s\\n", result5);
    free(result5);
    
    return 0;
}
