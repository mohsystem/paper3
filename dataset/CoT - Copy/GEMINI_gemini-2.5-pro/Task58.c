#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h> // For tolower

bool XO(const char* str) {
    if (str == NULL) {
        return true;
    }

    int xCount = 0;
    int oCount = 0;
    int len = strlen(str);

    for (int i = 0; i < len; i++) {
        char lower_c = tolower((unsigned char)str[i]);
        if (lower_c == 'x') {
            xCount++;
        } else if (lower_c == 'o') {
            oCount++;
        }
    }

    return xCount == oCount;
}

int main() {
    // Test cases
    printf("ooxx -> %s\n", XO("ooxx") ? "true" : "false");          // Expected: true
    printf("xooxx -> %s\n", XO("xooxx") ? "true" : "false");        // Expected: false
    printf("ooxXm -> %s\n", XO("ooxXm") ? "true" : "false");        // Expected: true
    printf("zpzpzpp -> %s\n", XO("zpzpzpp") ? "true" : "false");    // Expected: true
    printf("zzoo -> %s\n", XO("zzoo") ? "true" : "false");          // Expected: false
    return 0;
}