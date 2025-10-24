#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Validates a card string and increments the count for its suit.
 * 
 * @param card The card string to process.
 * @param suit_counts An array of 4 integers to store counts for S, H, D, C.
 * @return true if the card is valid and processed, false otherwise.
 */
bool countCardSuit(const char* card, int suit_counts[4]) {
    if (card == NULL) {
        return false;
    }
    size_t len = strlen(card);
    if (len < 3 || strchr(card, '_') == NULL) {
        return false;
    }
    
    char suit = card[len - 1];
    switch (suit) {
        case 'S': suit_counts[0]++; break;
        case 'H': suit_counts[1]++; break;
        case 'D': suit_counts[2]++; break;
        case 'C': suit_counts[3]++; break;
        default:
            return false; // Invalid suit
    }
    return true;
}

/**
 * @brief Determines if a flush can be formed from table and hand cards.
 * 
 * @param table_cards Array of 5 C-strings for table cards.
 * @param table_size Size of the table_cards array (must be 5).
 * @param hand_cards Array of 2 C-strings for hand cards.
 * @param hand_size Size of the hand_cards array (must be 2).
 * @return true if a flush exists, false otherwise.
 */
bool checkFlush(const char* table_cards[], size_t table_size, const char* hand_cards[], size_t hand_size) {
    if (table_cards == NULL || table_size != 5 || hand_cards == NULL || hand_size != 2) {
        return false;
    }

    int suit_counts[4] = {0, 0, 0, 0}; // S, H, D, C

    for (size_t i = 0; i < table_size; ++i) {
        if (!countCardSuit(table_cards[i], suit_counts)) {
            return false;
        }
    }

    for (size_t i = 0; i < hand_size; ++i) {
        if (!countCardSuit(hand_cards[i], suit_counts)) {
            return false;
        }
    }
    
    for (int i = 0; i < 4; ++i) {
        if (suit_counts[i] >= 5) {
            return true;
        }
    }

    return false;
}

int main() {
    // Test case 1
    const char* table1[] = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    const char* hand1[] = {"J_D", "3_D"};
    printf("Test 1: %s\n", checkFlush(table1, 5, hand1, 2) ? "true" : "false");

    // Test case 2
    const char* table2[] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    const char* hand2[] = {"K_S", "Q_S"};
    printf("Test 2: %s\n", checkFlush(table2, 5, hand2, 2) ? "true" : "false");

    // Test case 3
    const char* table3[] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    const char* hand3[] = {"3_S", "4_D"};
    printf("Test 3: %s\n", checkFlush(table3, 5, hand3, 2) ? "true" : "false");

    // Test case 4
    const char* table4[] = {"A_C", "K_C", "Q_C", "J_C", "10_C"};
    const char* hand4[] = {"2_H", "3_D"};
    printf("Test 4: %s\n", checkFlush(table4, 5, hand4, 2) ? "true" : "false");

    // Test case 5
    const char* table5[] = {"A_H", "A_S", "A_D", "A_C", "K_H"};
    const char* hand5[] = {"K_S", "K_D"};
    printf("Test 5: %s\n", checkFlush(table5, 5, hand5, 2) ? "true" : "false");

    return 0;
}