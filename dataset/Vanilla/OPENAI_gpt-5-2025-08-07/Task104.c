#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* handleInput(const char* input, size_t capacity) {
    if (input == NULL || capacity == 0) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t inlen = strlen(input);
    size_t n = capacity < inlen ? capacity : inlen;
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    if (n > 0) memcpy(out, input, n);
    out[n] = '\0';
    return out;
}

int main(void) {
    const char* inputs[] = {"Hello World", "", "Short", "ExactSize", "BoundaryTest"};
    size_t capacities[] = {5, 10, 10, 9, 0};
    for (int i = 0; i < 5; ++i) {
        char* out = handleInput(inputs[i], capacities[i]);
        if (!out) {
            printf("Case %d: [Allocation failed]\n", i + 1);
        } else {
            printf("Case %d: [%s]\n", i + 1, out);
            free(out);
        }
    }
    return 0;
}