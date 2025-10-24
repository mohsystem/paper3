#include <stdio.h>
#include <ctype.h>
#include <string.h>

static int suitIndex(const char* card) {
    if (card == NULL) return -1;
    const char* underscore = strrchr(card, '_');
    if (underscore == NULL || *(underscore + 1) == '\0') return -1;
    char ch = (char)toupper((unsigned char)*(underscore + 1));
    switch (ch) {
        case 'S': return 0;
        case 'H': return 1;
        case 'D': return 2;
        case 'C': return 3;
        default: return -1;
    }
}

int checkFlush(const char* table[], int tableLen, const char* hand[], int handLen) {
    int counts[4] = {0,0,0,0};
    int tableCounts[4] = {0,0,0,0};

    if (table == NULL || hand == NULL) return 0;

    for (int i = 0; i < tableLen; ++i) {
        int idx = suitIndex(table[i]);
        if (idx >= 0) {
            counts[idx]++;
            tableCounts[idx]++;
        }
    }

    int maxTable = 0;
    for (int i = 0; i < 4; ++i) {
        if (tableCounts[i] > maxTable) maxTable = tableCounts[i];
    }
    if (maxTable < 3) return 0;

    for (int i = 0; i < handLen; ++i) {
        int idx = suitIndex(hand[i]);
        if (idx >= 0) counts[idx]++;
    }

    for (int i = 0; i < 4; ++i) {
        if (counts[i] >= 5) return 1;
    }
    return 0;
}

int main(void) {
    const char* tables[5][5] = {
        {"A_S", "J_H", "7_D", "8_D", "10_D"},
        {"10_S", "7_S", "9_H", "4_S", "3_S"},
        {"3_S", "10_H", "10_D", "10_C", "10_S"},
        {"2_H", "5_C", "9_D", "K_H", "Q_H"},
        {"A_C", "2_D", "3_H", "4_S", "5_C"}
    };
    const char* hands[5][2] = {
        {"J_D", "3_D"},
        {"K_S", "Q_S"},
        {"3_S", "4_D"},
        {"A_H", "3_H"},
        {"6_D", "7_H"}
    };

    for (int i = 0; i < 5; ++i) {
        int result = checkFlush(tables[i], 5, hands[i], 2);
        printf("Test %d: %s\n", i + 1, result ? "true" : "false");
    }
    return 0;
}