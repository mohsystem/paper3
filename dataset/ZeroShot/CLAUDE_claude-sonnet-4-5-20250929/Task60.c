
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* expandedForm(int num) {
    if (num == 0) {
        char* result = (char*)malloc(2 * sizeof(char));
        strcpy(result, "0");
        return result;
    }
    
    char numStr[20];
    sprintf(numStr, "%d", num);
    int length = strlen(numStr);
    
    char* result = (char*)malloc(1000 * sizeof(char));
    result[0] = '\\0';
    
    for (int i = 0; i < length; i++) {
        if (numStr[i] != '0') {
            if (strlen(result) > 0) {
                strcat(result, " + ");
            }
            char temp[20];
            temp[0] = numStr[i];
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
    // Test cases
    char* result1 = expandedForm(12);
    printf("%s\\n", result1);      // "10 + 2"
    free(result1);
    
    char* result2 = expandedForm(42);
    printf("%s\\n", result2);      // "40 + 2"
    free(result2);
    
    char* result3 = expandedForm(70304);
    printf("%s\\n", result3);      // "70000 + 300 + 4"
    free(result3);
    
    char* result4 = expandedForm(9);
    printf("%s\\n", result4);      // "9"
    free(result4);
    
    char* result5 = expandedForm(123456);
    printf("%s\\n", result5);      // "100000 + 20000 + 3000 + 400 + 50 + 6"
    free(result5);
    
    return 0;
}
