#include <stdio.h>

char find_missing_letter(const char* array, int n) {
    for (int i = 1; i < n; i++) {
        if (array[i] != (char)(array[i - 1] + 1)) {
            return (char)(array[i - 1] + 1);
        }
    }
    return (char)(array[n - 1] + 1);
}

int main() {
    char arr1[] = {'a','b','c','d','f'};
    char arr2[] = {'O','Q','R','S'};
    char arr3[] = {'b','c','d','e','g'};
    char arr4[] = {'A','B','D'};
    char arr5[] = {'m','n','p','q','r'};

    printf("%c\n", find_missing_letter(arr1, sizeof(arr1)/sizeof(arr1[0])));
    printf("%c\n", find_missing_letter(arr2, sizeof(arr2)/sizeof(arr2[0])));
    printf("%c\n", find_missing_letter(arr3, sizeof(arr3)/sizeof(arr3[0])));
    printf("%c\n", find_missing_letter(arr4, sizeof(arr4)/sizeof(arr4[0])));
    printf("%c\n", find_missing_letter(arr5, sizeof(arr5)/sizeof(arr5[0])));
    return 0;
}