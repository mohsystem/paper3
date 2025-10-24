#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Determines if a flush is possible given 5 table cards and 2 hand cards.
 * A flush is 5 or more cards of the same suit.
 * 
 * @param table An array of 5 strings representing cards on the table.
 * @param hand An array of 2 strings representing cards in hand.
 * @return true if a flush exists, false otherwise.
 */
bool checkFlush(const char* table[5], const char* hand[2]) {
    int suitCounts[4] = {0, 0, 0, 0}; // 0:S, 1:H, 2:D, 3:C

    // Process table cards
    for (int i = 0; i < 5; i++) {
        const char* card = table[i];
        size_t len = strlen(card);
        if (len > 0) {
            char suit = card[len - 1];
            switch (suit) {
                case 'S': suitCounts[0]++; break;
                case 'H': suitCounts[1]++; break;
                case 'D': suitCounts[2]++; break;
                case 'C': suitCounts[3]++; break;
            }
        }
    }

    // Process hand cards
    for (int i = 0; i < 2; i++) {
        const char* card = hand[i];
        size_t len = strlen(card);
        if (len > 0) {
            char suit = card[len - 1];
            switch (suit) {
                case 'S': suitCounts[0]++; break;
                case 'H': suitCounts[1]++; break;
                case 'D': suitCounts[2]++; break;
                case 'C': suitCounts[3]++; break;
            }
        }
    }

    // Check for a flush
    for (int i = 0; i < 4; i++) {
        if (suitCounts[i] >= 5) {
            return true;
        }
    }

    return false;
}

int main() {
    // Test Case 1
    const char* table1[] = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    const char* hand1[] = {"J_D", "3_D"};
    printf("Test Case 1: %s\n", checkFlush(table1, hand1) ? "true" : "false");

    // Test Case 2
    const char* table2[] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    const char* hand2[] = {"K_S", "Q_S"};
    printf("Test Case 2: %s\n", checkFlush(table2, hand2) ? "true" : "false");

    // Test Case 3
    const char* table3[] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    const char* hand3[] = {"3_S", "4_D"};
    printf("Test Case 3: %s\n", checkFlush(table3, hand3) ? "true" : "false");

    // Test Case 4
    const char* table4[] = {"A_H", "A_S", "A_D", "A_C", "2_H"};
    const char* hand4[] = {"3_H", "4_H"};
    printf("Test Case 4: %s\n", checkFlush(table4, hand4) ? "true" : "false");

    // Test Case 5
    const char* table5[] = {"K_C", "Q_C", "J_C", "3_S", "4_H"};
    const char* hand5[] = {"10_C", "9_C"};
    printf("Test Case 5: %s\n", checkFlush(table5, hand5) ? "true" : "false");

    return 0;
}