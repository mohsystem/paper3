#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* tweakLetters(const char* s, const int* arr, int len) {
    char* out = (char*)malloc((len + 1) * sizeof(char));
    if (!out) return NULL;
    for (int i = 0; i < len; i++) {
        char ch = tolower((unsigned char)s[i]);
        if (i < len && ch >= 'a' && ch <= 'z') {
            int pos = ch - 'a';
            pos = (pos + arr[i]) % 26;
            if (pos < 0) pos += 26;
            out[i] = (char)('a' + pos);
        } else {
            out[i] = ch;
        }
    }
    out[len] = '\0';
    return out;
}

int main() {
    const char* words[5] = {"apple", "many", "rhino", "xyz", "zoo"};
    int arr0[] = {0, 1, -1, 0, -1};
    int arr1[] = {0, 0, 0, -1};
    int arr2[] = {1, 1, 1, 1, 1};
    int arr3[] = {1, 1, 1};
    int arr4[] = {1, 0, 0};
    const int* shifts[5] = {arr0, arr1, arr2, arr3, arr4};
    int lens[5] = {
        (int)strlen(words[0]),
        (int)strlen(words[1]),
        (int)strlen(words[2]),
        (int)strlen(words[3]),
        (int)strlen(words[4])
    };

    for (int i = 0; i < 5; i++) {
        char* res = tweakLetters(words[i], shifts[i], lens[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        }
    }
    return 0;
}