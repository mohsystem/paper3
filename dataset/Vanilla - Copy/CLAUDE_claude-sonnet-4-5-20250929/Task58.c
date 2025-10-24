
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool XO(const char* str) {
    if (str == NULL) {
        return true;
    }
    
    int xCount = 0;
    int oCount = 0;
    
    for (int i = 0; str[i] != '\\0'; i++) {
        char c = tolower(str[i]);
        if (c == 'x') {
            xCount++;
        } else if (c == 'o') {
            oCount++;
        }
    }
    
    return xCount == oCount;
}

int main() {
    // Test case 1
    printf("XO(\\"ooxx\\") = %s\\n", XO("ooxx") ? "true" : "false");
    
    // Test case 2
    printf("XO(\\"xooxx\\") = %s\\n", XO("xooxx") ? "true" : "false");
    
    // Test case 3
    printf("XO(\\"ooxXm\\") = %s\\n", XO("ooxXm") ? "true" : "false");
    
    // Test case 4
    printf("XO(\\"zpzpzpp\\") = %s\\n", XO("zpzpzpp") ? "true" : "false");
    
    // Test case 5
    printf("XO(\\"zzoo\\") = %s\\n", XO("zzoo") ? "true" : "false");
    
    return 0;
}
