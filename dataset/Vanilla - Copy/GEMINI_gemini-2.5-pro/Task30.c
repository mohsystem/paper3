#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* longest(const char* s1, const char* s2) {
    // A presence map for characters 'a' through 'z'.
    int char_map[26] = {0};

    // Mark characters present in s1.
    for (int i = 0; s1[i] != '\0'; i++) {
        if (s1[i] >= 'a' && s1[i] <= 'z') {
            char_map[s1[i] - 'a'] = 1;
        }
    }

    // Mark characters present in s2.
    for (int i = 0; s2[i] != '\0'; i++) {
        if (s2[i] >= 'a' && s2[i] <= 'z') {
            char_map[s2[i] - 'a'] = 1;
        }
    }

    // Allocate memory for the result string. Max possible length is 26.
    char* result = (char*)malloc(27 * sizeof(char));
    if (result == NULL) {
        // Handle memory allocation failure.
        return NULL;
    }

    int k = 0;
    // Build the result string. It will be sorted because we iterate from 'a' to 'z'.
    for (int i = 0; i < 26; i++) {
        if (char_map[i] == 1) {
            result[k++] = 'a' + i;
        }
    }
    result[k] = '\0'; // Null-terminate the string.

    return result;
}

int main() {
    // Test case 1
    char* res1 = longest("xyaabbbccccdefww", "xxxxyyyyabklmopq");
    printf("Test 1: %s\n", res1);
    free(res1);

    // Test case 2
    char* res2 = longest("abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz");
    printf("Test 2: %s\n", res2);
    free(res2);

    // Test case 3
    char* res3 = longest("aretheyhere", "yestheyarehere");
    printf("Test 3: %s\n", res3);
    free(res3);

    // Test case 4
    char* res4 = longest("loopingisfunbutdangerous", "lessdangerousthancoding");
    printf("Test 4: %s\n", res4);
    free(res4);

    // Test case 5
    char* res5 = longest("inmanylanguages", "theresapairoffunctions");
    printf("Test 5: %s\n", res5);
    free(res5);

    return 0;
}