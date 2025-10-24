
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool checkFlush(const char* table[], int tableSize, const char* hand[], int handSize) {
    // Input validation
    if (tableSize != 5 || handSize != 2) {
        return false;
    }
    
    // Count suits: S=0, H=1, D=2, C=3
    int suitCount[4] = {0, 0, 0, 0};
    
    // Process table cards
    for (int i = 0; i < tableSize; i++) {
        if (table[i] == NULL) {
            continue;
        }
        const char* underscore = strrchr(table[i], '_');
        if (underscore != NULL && *(underscore + 1) != '\\0') {
            char suit = *(underscore + 1);
            switch (suit) {
                case 'S': suitCount[0]++; break;
                case 'H': suitCount[1]++; break;
                case 'D': suitCount[2]++; break;
                case 'C': suitCount[3]++; break;
            }
        }
    }
    
    // Process hand cards
    for (int i = 0; i < handSize; i++) {
        if (hand[i] == NULL) {
            continue;
        }
        const char* underscore = strrchr(hand[i], '_');
        if (underscore != NULL && *(underscore + 1) != '\\0') {
            char suit = *(underscore + 1);
            switch (suit) {
                case 'S': suitCount[0]++; break;
                case 'H': suitCount[1]++; break;
                case 'D': suitCount[2]++; break;
                case 'C': suitCount[3]++; break;
            }
        }
    }
    
    // Check if any suit has at least 5 cards (flush)
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
    printf("%s\\n", checkFlush(table1, 5, hand1, 2) ? "true" : "false");
    
    // Test case 2: Spade flush
    const char* table2[] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    const char* hand2[] = {"K_S", "Q_S"};
    printf("%s\\n", checkFlush(table2, 5, hand2, 2) ? "true" : "false");
    
    // Test case 3: No flush
    const char* table3[] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    const char* hand3[] = {"3_S", "4_D"};
    printf("%s\\n", checkFlush(table3, 5, hand3, 2) ? "true" : "false");
    
    // Test case 4: Heart flush
    const char* table4[] = {"2_H", "5_H", "8_H", "K_H", "A_S"};
    const char* hand4[] = {"Q_H", "J_H"};
    printf("%s\\n", checkFlush(table4, 5, hand4, 2) ? "true" : "false");
    
    // Test case 5: Club flush
    const char* table5[] = {"9_C", "7_C", "4_C", "2_C", "K_H"};
    const char* hand5[] = {"6_C", "3_D"};
    printf("%s\\n", checkFlush(table5, 5, hand5, 2) ? "true" : "false");
    
    return 0;
}
