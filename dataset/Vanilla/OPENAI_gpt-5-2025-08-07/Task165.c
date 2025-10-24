#include <stdio.h>
#include <string.h>

static char getSuit(const char* card) {
    const char* p = strrchr(card, '_');
    if (p && *(p + 1)) return *(p + 1);
    return '?';
}

static int suitIndex(char s) {
    switch (s) {
        case 'S': return 0;
        case 'H': return 1;
        case 'D': return 2;
        case 'C': return 3;
        default: return -1;
    }
}

int checkFlush(const char* table[], int tableLen, const char* hand[], int handLen) {
    int cnt[4] = {0, 0, 0, 0}; // 0:S,1:H,2:D,3:C
    for (int i = 0; i < tableLen; ++i) {
        char s = getSuit(table[i]);
        int idx = suitIndex(s);
        if (idx >= 0) cnt[idx]++;
    }
    for (int i = 0; i < handLen; ++i) {
        char s = getSuit(hand[i]);
        int idx = suitIndex(s);
        if (idx >= 0) cnt[idx]++;
    }
    for (int i = 0; i < 4; ++i) {
        if (cnt[i] >= 5) return 1;
    }
    return 0;
}

int main() {
    const char* t1[] = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    const char* h1[] = {"J_D", "3_D"};
    printf("%s\n", checkFlush(t1, 5, h1, 2) ? "true" : "false");

    const char* t2[] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    const char* h2[] = {"K_S", "Q_S"};
    printf("%s\n", checkFlush(t2, 5, h2, 2) ? "true" : "false");

    const char* t3[] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    const char* h3[] = {"3_S", "4_D"};
    printf("%s\n", checkFlush(t3, 5, h3, 2) ? "true" : "false");

    const char* t4[] = {"2_H", "5_H", "7_H", "9_H", "K_H"};
    const char* h4[] = {"A_S", "3_D"};
    printf("%s\n", checkFlush(t4, 5, h4, 2) ? "true" : "false");

    const char* t5[] = {"2_C", "5_C", "7_C", "9_H", "K_D"};
    const char* h5[] = {"A_C", "3_D"};
    printf("%s\n", checkFlush(t5, 5, h5, 2) ? "true" : "false");

    return 0;
}