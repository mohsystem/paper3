#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static int suit_index(const char *card) {
    if (card == NULL) return -1;
    const char *first = strchr(card, '_');
    if (first == NULL || first == card) return -1;
    const char *second = strchr(first + 1, '_');
    if (second != NULL) return -1; // more than one underscore
    size_t len = strlen(card);
    size_t pos = (size_t)(first - card);
    if (pos + 1 >= len) return -1; // no suit char
    if (len - pos - 1 != 1) return -1; // suit must be single char
    char s = card[pos + 1];
    switch (s) {
        case 'S': return 0;
        case 'H': return 1;
        case 'D': return 2;
        case 'C': return 3;
        default: return -1;
    }
}

bool checkFlush(const char *table[5], const char *hand[2]) {
    if (table == NULL || hand == NULL) return false;

    int tableCounts[4] = {0, 0, 0, 0};
    for (int i = 0; i < 5; i++) {
        int idx = suit_index(table[i]);
        if (idx < 0) return false;
        tableCounts[idx]++;
    }

    bool possible = false;
    for (int i = 0; i < 4; i++) {
        if (tableCounts[i] >= 3) { possible = true; break; }
    }
    if (!possible) return false;

    int totalCounts[4];
    for (int i = 0; i < 4; i++) totalCounts[i] = tableCounts[i];

    for (int i = 0; i < 2; i++) {
        int idx = suit_index(hand[i]);
        if (idx < 0) return false;
        totalCounts[idx]++;
    }

    for (int i = 0; i < 4; i++) {
        if (totalCounts[i] >= 5) return true;
    }
    return false;
}

int main(void) {
    // Test case 1: diamond flush
    const char *t1[5] = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    const char *h1[2] = {"J_D", "3_D"};
    printf("%s\n", checkFlush(t1, h1) ? "true" : "false"); // true

    // Test case 2: spade flush
    const char *t2[5] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    const char *h2[2] = {"K_S", "Q_S"};
    printf("%s\n", checkFlush(t2, h2) ? "true" : "false"); // true

    // Test case 3: no flush
    const char *t3[5] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    const char *h3[2] = {"3_S", "4_D"};
    printf("%s\n", checkFlush(t3, h3) ? "true" : "false"); // false

    // Test case 4: clubs flush using 3 on table + 2 in hand
    const char *t4[5] = {"2_C", "5_C", "9_C", "K_H", "A_D"};
    const char *h4[2] = {"3_C", "7_C"};
    printf("%s\n", checkFlush(t4, h4) ? "true" : "false"); // true

    // Test case 5: four hearts on table, none in hand -> no flush
    const char *t5[5] = {"2_H", "4_H", "6_H", "8_H", "J_D"};
    const char *h5[2] = {"Q_S", "K_C"};
    printf("%s\n", checkFlush(t5, h5) ? "true" : "false"); // false

    return 0;
}