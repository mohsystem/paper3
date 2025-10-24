#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Checks if a string has the same amount of 'x's and 'o's, case-insensitively.
 *
 * @param str The string to check. The string can contain any character.
 * @return true if the count of 'x's and 'o's is equal, false otherwise.
 */
bool XO(const char *str) {
    // Secure code: check for NULL pointer to prevent crashes.
    if (str == NULL) {
        return true; // No 'x's or 'o's
    }

    int xCount = 0;
    int oCount = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        // Use cast to unsigned char for safe conversion in tolower
        char lower_ch = tolower((unsigned char)str[i]);
        if (lower_ch == 'x') {
            xCount++;
        } else if (lower_ch == 'o') {
            oCount++;
        }
    }
    
    return xCount == oCount;
}

int main() {
    // Test cases
    printf("XO(\"ooxx\") => %s\n", XO("ooxx") ? "true" : "false");
    printf("XO(\"xooxx\") => %s\n", XO("xooxx") ? "true" : "false");
    printf("XO(\"ooxXm\") => %s\n", XO("ooxXm") ? "true" : "false");
    printf("XO(\"zpzpzpp\") => %s\n", XO("zpzpzpp") ? "true" : "false");
    printf("XO(\"zzoo\") => %s\n", XO("zzoo") ? "true" : "false");

    return 0;
}