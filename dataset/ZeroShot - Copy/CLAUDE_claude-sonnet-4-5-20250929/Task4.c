
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool isIsogram(const char* str) {
    if (str == NULL || str[0] == '\\0') {
        return true;
    }
    
    int len = strlen(str);
    char lowerStr[len + 1];
    
    for (int i = 0; i < len; i++) {
        lowerStr[i] = tolower(str[i]);
    }
    lowerStr[len] = '\\0';
    
    for (int i = 0; i < len; i++) {
        for (int j = i + 1; j < len; j++) {
            if (lowerStr[i] == lowerStr[j]) {
                return false;
            }
        }
    }
    
    return true;
}

int main() {
    // Test cases
    printf("%s\\n", isIsogram("Dermatoglyphics") ? "true" : "false");  // true
    printf("%s\\n", isIsogram("aba") ? "true" : "false");              // false
    printf("%s\\n", isIsogram("moOse") ? "true" : "false");            // false
    printf("%s\\n", isIsogram("") ? "true" : "false");                 // true
    printf("%s\\n", isIsogram("isogram") ? "true" : "false");          // false
    
    return 0;
}
