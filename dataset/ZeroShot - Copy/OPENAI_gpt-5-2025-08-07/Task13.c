#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

bool isValidWalk(const char* walk[], size_t len) {
    if (walk == NULL || len != 10) return false;
    int x = 0, y = 0;
    for (size_t i = 0; i < len; ++i) {
        const char* dir = walk[i];
        if (dir == NULL || strlen(dir) != 1) return false;
        char c = dir[0];
        switch (c) {
            case 'n': y += 1; break;
            case 's': y -= 1; break;
            case 'e': x += 1; break;
            case 'w': x -= 1; break;
            default: return false;
        }
    }
    return x == 0 && y == 0;
}

int main(void) {
    const char* t1[] = {"n","s","n","s","n","s","n","s","n","s"};
    const char* t2[] = {"w","e","w","e","w","e","w","e","w","e","w","e"};
    const char* t3[] = {"w","w","w","w","w","w","w","w","w","w"};
    const char* t4[] = {"n","s","n","s","n","s","n","s","n","x"};
    const char* t5[] = {"n","n","e","e","s","s","w","w","n","s"};

    bool expected[] = {true, false, false, false, true};
    const char** tests[] = {t1, t2, t3, t4, t5};
    size_t lengths[] = {10, 12, 10, 10, 10};

    for (size_t i = 0; i < 5; ++i) {
        bool result = isValidWalk(tests[i], lengths[i]);
        printf("Test %zu: %s (expected %s)\n", i + 1, result ? "true" : "false", expected[i] ? "true" : "false");
    }
    return 0;
}