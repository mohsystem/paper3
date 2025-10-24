#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* tweakLetters(const char* s, const int* shifts, size_t n) {
    if (s == NULL || shifts == NULL) {
        return NULL;
    }
    size_t len = strlen(s);
    if (len != n) {
        return NULL;
    }
    char* out = (char*)malloc(len + 1);
    if (out == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        char ch = s[i];
        if (ch >= 'a' && ch <= 'z') {
            int base = ch - 'a';
            int adj = shifts[i] % 26;
            if (adj < 0) adj += 26;
            int newIdx = (base + adj) % 26;
            out[i] = (char)('a' + newIdx);
        } else {
            out[i] = ch;
        }
    }
    out[len] = '\0';
    return out;
}

int main(void) {
    {
        const char* s = "apple";
        int arr[] = {0, 1, -1, 0, -1};
        char* res = tweakLetters(s, arr, sizeof(arr)/sizeof(arr[0]));
        if (res) { printf("%s\n", res); free(res); } else { printf("Error\n"); }
    }
    {
        const char* s = "many";
        int arr[] = {0, 0, 0, -1};
        char* res = tweakLetters(s, arr, sizeof(arr)/sizeof(arr[0]));
        if (res) { printf("%s\n", res); free(res); } else { printf("Error\n"); }
    }
    {
        const char* s = "rhino";
        int arr[] = {1, 1, 1, 1, 1};
        char* res = tweakLetters(s, arr, sizeof(arr)/sizeof(arr[0]));
        if (res) { printf("%s\n", res); free(res); } else { printf("Error\n"); }
    }
    {
        const char* s = "zzz";
        int arr[] = {1, 1, 1};
        char* res = tweakLetters(s, arr, sizeof(arr)/sizeof(arr[0]));
        if (res) { printf("%s\n", res); free(res); } else { printf("Error\n"); }
    }
    {
        const char* s = "abc";
        int arr[] = {27, -28, 52};
        char* res = tweakLetters(s, arr, sizeof(arr)/sizeof(arr[0]));
        if (res) { printf("%s\n", res); free(res); } else { printf("Error\n"); }
    }
    return 0;
}