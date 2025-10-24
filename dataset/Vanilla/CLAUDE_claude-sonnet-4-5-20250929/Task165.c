
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool checkFlush(char* table[], int tableSize, char* hand[], int handSize) {
    int suitCount[4] = {0}; // S, H, D, C
    
    // Count suits from table
    for (int i = 0; i < tableSize; i++) {
        char* underscore = strchr(table[i], '_');
        char suit = *(underscore + 1);
        
        if (suit == 'S') suitCount[0]++;
        else if (suit == 'H') suitCount[1]++;
        else if (suit == 'D') suitCount[2]++;
        else if (suit == 'C') suitCount[3]++;
    }
    
    // Check each suit
    for (int i = 0; i < 4; i++) {
        if (suitCount[i] >= 3) {
            char targetSuit;
            if (i == 0) targetSuit = 'S';
            else if (i == 1) targetSuit = 'H';
            else if (i == 2) targetSuit = 'D';
            else targetSuit = 'C';
            
            int handCount = 0;
            for (int j = 0; j < handSize; j++) {
                char* underscore = strchr(hand[j], '_');
                char suit = *(underscore + 1);
                if (suit == targetSuit) {
                    handCount++;
                }
            }
            
            if (suitCount[i] + handCount >= 5) {
                return true;
            }
        }
    }
    
    return false;
}

int main() {
    // Test case 1
    char* table1[] = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    char* hand1[] = {"J_D", "3_D"};
    printf("%s\\n", checkFlush(table1, 5, hand1, 2) ? "true" : "false");
    
    // Test case 2
    char* table2[] = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    char* hand2[] = {"K_S", "Q_S"};
    printf("%s\\n", checkFlush(table2, 5, hand2, 2) ? "true" : "false");
    
    // Test case 3
    char* table3[] = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    char* hand3[] = {"3_S", "4_D"};
    printf("%s\\n", checkFlush(table3, 5, hand3, 2) ? "true" : "false");
    
    // Test case 4
    char* table4[] = {"2_H", "5_H", "8_H", "K_C", "A_S"};
    char* hand4[] = {"9_H", "J_H"};
    printf("%s\\n", checkFlush(table4, 5, hand4, 2) ? "true" : "false");
    
    // Test case 5
    char* table5[] = {"2_C", "3_C", "4_C", "5_H", "6_H"};
    char* hand5[] = {"7_H", "8_D"};
    printf("%s\\n", checkFlush(table5, 5, hand5, 2) ? "true" : "false");
    
    return 0;
}
