#include <stdio.h>

int isValidWalk(const char* walk, int len) {
    if (len != 10) return 0;
    int x = 0, y = 0;
    for (int i = 0; i < len; i++) {
        char c = walk[i];
        switch (c) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
            default: return 0;
        }
    }
    return (x == 0 && y == 0) ? 1 : 0;
}

int main() {
    char w1[10] = {'n','s','n','s','e','w','e','w','n','s'}; // true
    char w2[3]  = {'n','s','e'}; // false
    char w3[10] = {'n','n','n','n','n','s','s','s','e','w'}; // false
    char w4[10] = {'e','w','e','w','n','s','n','s','n','s'}; // true
    char w5[10] = {'n','n','n','n','n','n','n','n','n','n'}; // false

    printf("%s\n", isValidWalk(w1, 10) ? "true" : "false");
    printf("%s\n", isValidWalk(w2, 3)  ? "true" : "false");
    printf("%s\n", isValidWalk(w3, 10) ? "true" : "false");
    printf("%s\n", isValidWalk(w4, 10) ? "true" : "false");
    printf("%s\n", isValidWalk(w5, 10) ? "true" : "false");

    return 0;
}