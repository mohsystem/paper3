#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool XO(const char* str) {
    int xCount = 0;
    int oCount = 0;
    if (str == NULL) {
        return true;
    }
    for (int i = 0; str[i] != '\0'; i++) {
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
    const char* testCases[] = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
    int numTests = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < numTests; i++) {
        printf("XO(\"%s\") => %s\n", testCases[i], XO(testCases[i]) ? "true" : "false");
    }
    return 0;
}