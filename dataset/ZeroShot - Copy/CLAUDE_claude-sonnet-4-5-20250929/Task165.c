
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool checkFlush(char table[][10], int tableSize, char hand[][10], int handSize) {
    // Count suits on table
    int suitCount[4] = {0}; // S=0, H=1, D=2, C=3
    
    for (int i = 0; i < tableSize; i++) {
        char suit = table[i][strlen(table[i]) - 1];
        int index = -1;
        switch(suit) {
            case 'S': index = 0; break;
            case 'H': index = 1; break;
            case 'D': index = 2; break;
            case 'C': index = 3; break;
        }
        if (index >= 0) suitCount[index]++;
    }
    
    // Check each suit that has at least 3 cards on table
    char suits[] = {'S', 'H', 'D', 'C'};
    for (int i = 0; i < 4; i++) {
        if (suitCount[i] >= 3) {
            char targetSuit = suits[i];
            int totalCount = suitCount[i];
            
            // Count matching suits in hand
            for (int j = 0; j < handSize; j++) {
                char suit = hand[j][strlen(hand[j]) - 1];
                if (suit == targetSuit) {
                    totalCount++;
                }
            }
            
            // Check if we have 5 or more cards of same suit
            if (totalCount >= 5) {
                return true;
            }
        }
    }
    
    return false;
}

int main() {
    // Test case 1
    char table1[][10] = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    char hand1[][10] = {"J_D", "3_D"};
    printf("%s\\n", checkFlush(table1, 5, hand1, 2) ? "true" : "false");
    
    // Test case 2
    char table2[][10] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    char hand2[][10] = {"K_S", "Q_S"};
    printf("%s\\n", checkFlush(table2, 5, hand2, 2) ? "true" : "false");
    
    // Test case 3
    char table3[][10] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    char hand3[][10] = {"3_S", "4_D"};
    printf("%s\\n", checkFlush(table3, 5, hand3, 2) ? "true" : "false");
    
    // Test case 4
    char table4[][10] = {"2_H", "5_H", "9_H", "K_H", "A_S"};
    char hand4[][10] = {"3_H", "7_C"};
    printf("%s\\n", checkFlush(table4, 5, hand4, 2) ? "true" : "false");
    
    // Test case 5
    char table5[][10] = {"2_C", "3_C", "4_C", "5_S", "6_H"};
    char hand5[][10] = {"7_D", "8_S"};
    printf("%s\\n", checkFlush(table5, 5, hand5, 2) ? "true" : "false");
    
    return 0;
}
