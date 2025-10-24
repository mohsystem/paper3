
#include <stdio.h>
#include <string.h>

const char* oddOrEven(int* array, int size) {
    if (array == NULL || size == 0) {
        return "even";
    }
    
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    
    return (sum % 2 == 0) ? "even" : "odd";
}

int main() {
    int test1[] = {0};
    int test2[] = {0, 1, 4};
    int test3[] = {0, -1, -5};
    int test4[] = {};
    int test5[] = {1, 2, 3, 4, 5};
    
    printf("%s\\n", oddOrEven(test1, 1));
    printf("%s\\n", oddOrEven(test2, 3));
    printf("%s\\n", oddOrEven(test3, 3));
    printf("%s\\n", oddOrEven(test4, 0));
    printf("%s\\n", oddOrEven(test5, 5));
    
    return 0;
}
