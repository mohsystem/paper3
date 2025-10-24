/* Task168 - C implementation
   Chain-of-Through process in code generation:
   1) Problem: Shift lowercase letters by array offsets with wrap-around.
   2) Security: Validate lengths, handle NULLs, prevent overflow.
   3) Secure coding: Use size_t, check allocations, normalize modulo.
   4) Review: Ensure correct negative modulo handling.
   5) Output: Final function and 5 test cases.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* tweakLetters(const char* s, const int* arr, size_t arrLen) {
    if (s == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) {
        return NULL; // allocation failed
    }
    for (size_t i = 0; i < n; ++i) {
        char ch = s[i];
        if (ch >= 'a' && ch <= 'z' && i < arrLen && arr != NULL) {
            int base = ch - 'a';
            int shift = arr[i] % 26;
            if (shift < 0) shift += 26;
            int ni = (base + shift) % 26;
            out[i] = (char)('a' + ni);
        } else if (ch >= 'a' && ch <= 'z') {
            out[i] = ch;
        } else {
            out[i] = ch;
        }
    }
    out[n] = '\0';
    return out;
}

int main(void) {
    int arr1[] = {0, 1, -1, 0, -1};
    char* r1 = tweakLetters("apple", arr1, sizeof(arr1)/sizeof(arr1[0]));
    printf("%s\n", r1); // aqold
    free(r1);

    int arr2[] = {0, 0, 0, -1};
    char* r2 = tweakLetters("many", arr2, sizeof(arr2)/sizeof(arr2[0]));
    printf("%s\n", r2); // manx
    free(r2);

    int arr3[] = {1, 1, 1, 1, 1};
    char* r3 = tweakLetters("rhino", arr3, sizeof(arr3)/sizeof(arr3[0]));
    printf("%s\n", r3); // sijop
    free(r3);

    int arr4[] = {1, 1, 1};
    char* r4 = tweakLetters("zzz", arr4, sizeof(arr4)/sizeof(arr4[0]));
    printf("%s\n", r4); // aaa
    free(r4);

    int arr5[] = {-1, -1, -1};
    char* r5 = tweakLetters("abc", arr5, sizeof(arr5)/sizeof(arr5[0]));
    printf("%s\n", r5); // zab
    free(r5);

    return 0;
}