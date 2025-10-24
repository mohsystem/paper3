
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool XO(const char* str) {
    if (str == NULL) {
        return true;
    }
    
    int countX = 0;
    int countO = 0;
    size_t len = strlen(str);
    
    for (size_t i = 0; i < len; i++) {
        char c = tolower((unsigned char)str[i]);
        if (c == 'x') {
            countX++;
        } else if (c == 'o') {
            countO++;
        }
    }
    
    return countX == countO;
}

int main() {
    printf("Test 1: XO(\\"ooxx\\") = %s\\n", XO("ooxx") ? "true" : "false");
    printf("Test 2: XO(\\"xooxx\\") = %s\\n", XO("xooxx") ? "true" : "false");
    printf("Test 3: XO(\\"ooxXm\\") = %s\\n", XO("ooxXm") ? "true" : "false");
    printf("Test 4: XO(\\"zpzpzpp\\") = %s\\n", XO("zpzpzpp") ? "true" : "false");
    printf("Test 5: XO(\\"zzoo\\") = %s\\n", XO("zzoo") ? "true" : "false");
    
    return 0;
}
