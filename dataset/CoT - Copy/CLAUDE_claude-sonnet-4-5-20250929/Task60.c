
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* expandedForm(int num) {
    if (num <= 0) {
        return NULL;
    }
    
    char numStr[20];
    snprintf(numStr, sizeof(numStr), "%d", num);
    int length = strlen(numStr);
    
    char* result = (char*)malloc(1000 * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    result[0] = '\\0';
    
    int first = 1;
    for (int i = 0; i < length; i++) {
        if (numStr[i] != '0') {
            if (!first) {
                strcat(result, " + ");
            }
            
            char temp[100];
            temp[0] = numStr[i];
            temp[1] = '\\0';
            
            for (int j = i + 1; j < length; j++) {
                strcat(temp, "0");
            }
            
            strcat(result, temp);
            first = 0;
        }
    }
    
    return result;
}

int main() {
    // Test cases
    char* result1 = expandedForm(12);
    printf("%s\\n", result1);
    free(result1);
    
    char* result2 = expandedForm(42);
    printf("%s\\n", result2);
    free(result2);
    
    char* result3 = expandedForm(70304);
    printf("%s\\n", result3);
    free(result3);
    
    char* result4 = expandedForm(9);
    printf("%s\\n", result4);
    free(result4);
    
    char* result5 = expandedForm(100000);
    printf("%s\\n", result5);
    free(result5);
    
    return 0;
}
