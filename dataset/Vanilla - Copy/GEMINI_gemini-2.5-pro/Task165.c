#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Determines if a 5-card flush can be made from 5 table cards and 2 hand cards.
 * 
 * @param tableCards An array of 5 strings representing cards on the table.
 * @param handCards An array of 2 strings representing cards in hand.
 * @return true if a flush exists, false otherwise.
 */
bool checkFlush(const char* tableCards[5], const char* handCards[2]) {
    // Index 0: Spades, 1: Hearts, 2: Diamonds, 3: Clubs
    int suitCounts[4] = {0, 0, 0, 0};

    // Process table cards
    for (int i = 0; i < 5; i++) {
        char suit = tableCards[i][strlen(tableCards[i]) - 1];
        switch (suit) {
            case 'S': suitCounts[0]++; break;
            case 'H': suitCounts[1]++; break;
            case 'D': suitCounts[2]++; break;
            case 'C': suitCounts[3]++; break;
        }
    }
    
    // Process hand cards
    for (int i = 0; i < 2; i++) {
        char suit = handCards[i][strlen(handCards[i]) - 1];
        switch (suit) {
            case 'S': suitCounts[0]++; break;
            case 'H': suitCounts[1]++; break;
            case 'D': suitCounts[2]++; break;
            case 'C': suitCounts[3]++; break;
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
    const char* table1[5] = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    const char* hand1[2] = {"J_D", "3_D"};
    printf("%s\n", checkFlush(table1, hand1) ? "true" : "false");

    // Test Case 2
    const char* table2[5] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    const char* hand2[2] = {"K_S", "Q_S"};
    printf("%s\n", checkFlush(table2, hand2) ? "true" : "false");

    // Test Case 3
    const char* table3[5] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    const char* hand3[2] = {"3_S", "4_D"};
    printf("%s\n", checkFlush(table3, hand3) ? "true" : "false");

    // Test Case 4
    const char* table4[5] = {"A_H", "K_H", "Q_H", "J_H", "10_H"};
    const char* hand4[2] = {"3_S", "4_D"};
    printf("%s\n", checkFlush(table4, hand4) ? "true" : "false");

    // Test Case 5
    const char* table5[5] = {"A_S", "K_S", "Q_S", "J_S", "2_H"};
    const char* hand5[2] = {"3_D", "4_C"};
    printf("%s\n", checkFlush(table5, hand5) ? "true" : "false");

    return 0;
}