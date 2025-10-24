
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
    
    for (int i = 0; str[i] != '\\0'; i++) {
        char lowerCh = tolower(str[i]);
        if (lowerCh == 'x') {
            countX++;
        } else if (lowerCh == 'o') {
            countO++;
        }
    }
    
    return countX == countO;
}

int main() {
    printf("Test 1: XO(\\"ooxx\\") = %s (expected: true)\\n", XO("ooxx") ? "true" : "false");
    printf("Test 2: XO(\\"xooxx\\") = %s (expected: false)\\n", XO("xooxx") ? "true" : "false");
    printf("Test 3: XO(\\"ooxXm\\") = %s (expected: true)\\n", XO("ooxXm") ? "true" : "false");
    printf("Test 4: XO(\\"zpzpzpp\\") = %s (expected: true)\\n", XO("zpzpzpp") ? "true" : "false");
    printf("Test 5: XO(\\"zzoo\\") = %s (expected: false)\\n", XO("zzoo") ? "true" : "false");
    
    return 0;
}
