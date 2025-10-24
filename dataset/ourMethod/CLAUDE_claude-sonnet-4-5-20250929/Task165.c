
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Maximum card string length including null terminator
#define MAX_CARD_LEN 32

// Function to check if there exists a flush (5 cards of the same suit)
// Returns true if a flush exists, false otherwise
bool checkFlush(const char* table[], int tableSize, const char* hand[], int handSize) {
    // Input validation: check array sizes
    if (tableSize != 5 || handSize != 2) {
        return false;  // Invalid input, fail closed
    }
    
    // Validate all pointers are not NULL
    if (table == NULL || hand == NULL) {
        return false;  // NULL pointer, fail closed
    }
    
    // Suit counters: index 0=S, 1=H, 2=D, 3=C
    int suitCount[4] = {0, 0, 0, 0};
    
    // Process table cards
    for (int i = 0; i < tableSize; i++) {
        if (table[i] == NULL) {
            return false;  // NULL card pointer, fail closed
        }
        
        // Validate card string length to prevent buffer overruns
        size_t len = strnlen(table[i], MAX_CARD_LEN);
        if (len < 3 || len >= MAX_CARD_LEN) {
            return false;  // Invalid card length, fail closed
        }
        
        // Find underscore position
        const char* underscore = strchr(table[i], '_');
        if (underscore == NULL || underscore[1] == '\\0') {
            return false;  // Invalid format, fail closed
        }
        
        // Extract suit character
        char suit = underscore[1];
        
        // Map suit to index and increment counter
        if (suit == 'S') {
            suitCount[0]++;
        } else if (suit == 'H') {
            suitCount[1]++;
        } else if (suit == 'D') {
            suitCount[2]++;
        } else if (suit == 'C') {
            suitCount[3]++;
        } else {
            return false;  // Invalid suit character, fail closed
        }
    }
    
    // Process hand cards
    for (int i = 0; i < handSize; i++) {
        if (hand[i] == NULL) {
            return false;  // NULL card pointer, fail closed
        }
        
        // Validate card string length
        size_t len = strnlen(hand[i], MAX_CARD_LEN);
        if (len < 3 || len >= MAX_CARD_LEN) {
            return false;  // Invalid card length, fail closed
        }
        
        // Find underscore position
        const char* underscore = strchr(hand[i], '_');
        if (underscore == NULL || underscore[1] == '\\0') {
            return false;  // Invalid format, fail closed
        }
        
        // Extract suit character
        char suit = underscore[1];
        
        // Map suit to index and increment counter
        if (suit == 'S') {
            suitCount[0]++;
        } else if (suit == 'H') {
            suitCount[1]++;
        } else if (suit == 'D') {
            suitCount[2]++;
        } else if (suit == 'C') {
            suitCount[3]++;
        } else {
            return false;  // Invalid suit character, fail closed
        }
    }
    
    // Check if any suit has 5 or more cards (flush)
    for (int i = 0; i < 4; i++) {
        if (suitCount[i] >= 5) {
            return true;
        }
    }
    
    return false;
}

int main() {
    // Test case 1: Diamond flush
    const char* table1[] = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    const char* hand1[] = {"J_D", "3_D"};
    printf("Test 1: %s\\n", checkFlush(table1, 5, hand1, 2) ? "true" : "false");
    
    // Test case 2: Spade flush
    const char* table2[] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    const char* hand2[] = {"K_S", "Q_S"};
    printf("Test 2: %s\\n", checkFlush(table2, 5, hand2, 2) ? "true" : "false");
    
    // Test case 3: No flush
    const char* table3[] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    const char* hand3[] = {"3_S", "4_D"};
    printf("Test 3: %s\\n", checkFlush(table3, 5, hand3, 2) ? "true" : "false");
    
    // Test case 4: Heart flush
    const char* table4[] = {"2_H", "5_H", "9_H", "K_H", "3_C"};
    const char* hand4[] = {"A_H", "7_D"};
    printf("Test 4: %s\\n", checkFlush(table4, 5, hand4, 2) ? "true" : "false");
    
    // Test case 5: Club flush
    const char* table5[] = {"6_C", "8_C", "9_C", "J_C", "2_D"};
    const char* hand5[] = {"K_C", "A_S"};
    printf("Test 5: %s\\n", checkFlush(table5, 5, hand5, 2) ? "true" : "false");
    
    return 0;
}
