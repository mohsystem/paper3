
#include <iostream>
#include <string>
#include <map>
#include <vector>

bool checkFlush(std::vector<std::string> table, std::vector<std::string> hand) {
    std::map<char, int> suitCount;
    suitCount['S'] = 0;
    suitCount['H'] = 0;
    suitCount['D'] = 0;
    suitCount['C'] = 0;
    
    // Count suits from table
    for (const std::string& card : table) {
        size_t pos = card.find('_');
        char suit = card[pos + 1];
        suitCount[suit]++;
    }
    
    // Check each suit
    for (auto& pair : suitCount) {
        if (pair.second >= 3) {
            char targetSuit = pair.first;
            int handCount = 0;
            
            // Check if hand has cards of this suit
            for (const std::string& card : hand) {
                size_t pos = card.find('_');
                char suit = card[pos + 1];
                if (suit == targetSuit) {
                    handCount++;
                }
            }
            
            // Need at least 5 total cards of same suit
            if (pair.second + handCount >= 5) {
                return true;
            }
        }
    }
    
    return false;
}

int main() {
    // Test case 1
    std::cout << std::boolalpha << checkFlush(
        {"A_S", "J_H", "7_D", "8_D", "10_D"}, 
        {"J_D", "3_D"}
    ) << std::endl; // true - diamond flush
    
    // Test case 2
    std::cout << checkFlush(
        {"10_S", "7_S", "9_H", "4_S", "3_S"}, 
        {"K_S", "Q_S"}
    ) << std::endl; // true - spade flush
    
    // Test case 3
    std::cout << checkFlush(
        {"3_S", "10_H", "10_D", "10_C", "10_S"}, 
        {"3_S", "4_D"}
    ) << std::endl; // false
    
    // Test case 4
    std::cout << checkFlush(
        {"2_H", "5_H", "8_H", "K_C", "A_S"}, 
        {"9_H", "J_H"}
    ) << std::endl; // true - heart flush
    
    // Test case 5
    std::cout << checkFlush(
        {"2_C", "3_C", "4_C", "5_H", "6_H"}, 
        {"7_H", "8_D"}
    ) << std::endl; // false
    
    return 0;
}
