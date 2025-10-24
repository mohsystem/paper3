#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Step 1: Problem understanding and function definition
int checkFlush(const char* table[], int tableLen, const char* hand[], int handLen) {
    int counts[4] = {0, 0, 0, 0}; // 0:S, 1:H, 2:D, 3:C

    // Step 2-4: Secure parsing and counting with input validation
    auto int suit_index_from_card(const char* card) {
        if (card == NULL) return -1;
        const char* p = strchr(card, '_');
        if (p == NULL || *(p + 1) == '\0') return -1;
        char s = (char)toupper((unsigned char)*(p + 1));
        switch (s) {
            case 'S': return 0;
            case 'H': return 1;
            case 'D': return 2;
            case 'C': return 3;
            default:  return -1;
        }
    }

    for (int i = 0; i < tableLen; ++i) {
        int si = suit_index_from_card(table[i]);
        if (si >= 0) counts[si]++;
    }
    for (int i = 0; i < handLen; ++i) {
        int si = suit_index_from_card(hand[i]);
        if (si >= 0) counts[si]++;
    }

    for (int i = 0; i < 4; ++i) {
        if (counts[i] >= 5) return 1;
    }
    return 0;
}

// Step 5: Main with 5 test cases
int main(void) {
    const char* table1[] = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    const char* hand1[]  = {"J_D", "3_D"};

    const char* table2[] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    const char* hand2[]  = {"K_S", "Q_S"};

    const char* table3[] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    const char* hand3[]  = {"3_S", "4_D"};

    const char* table4[] = {"2_H", "5_H", "K_H", "9_C", "J_D"};
    const char* hand4[]  = {"A_H", "3_H"};

    const char* table5[] = {"a_s", "j_h", "7_d", "8_d", "10_d"};
    const char* hand5[]  = {"j_d", "3_c"};

    const char** tables[] = {table1, table2, table3, table4, table5};
    const char** hands[]  = {hand1, hand2, hand3, hand4, hand5};
    int tLens[] = {5, 5, 5, 5, 5};
    int hLens[] = {2, 2, 2, 2, 2};

    for (int i = 0; i < 5; ++i) {
        int res = checkFlush(tables[i], tLens[i], hands[i], hLens[i]);
        printf("Test %d: %s\n", i + 1, res ? "true" : "false");
    }

    return 0;
}