
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstring>

// Function to check if there exists a flush (5 cards of the same suit)
// Returns true if a flush exists, false otherwise
bool checkFlush(const std::vector<std::string>& table, const std::vector<std::string>& hand) {
    // Input validation: check array sizes
    if (table.size() != 5 || hand.size() != 2) {
        return false;  // Invalid input, fail closed
    }
    
    // Map to count suits: S, H, D, C
    std::unordered_map<char, int> suitCount;
    suitCount['S'] = 0;
    suitCount['H'] = 0;
    suitCount['D'] = 0;
    suitCount['C'] = 0;
    
    // Process table cards
    for (size_t i = 0; i < table.size(); ++i) {
        const std::string& card = table[i];
        
        // Validate card format: must contain underscore and be at least 3 chars
        if (card.length() < 3) {
            return false;  // Invalid card format, fail closed
        }
        
        size_t underscorePos = card.find('_');
        if (underscorePos == std::string::npos || underscorePos + 1 >= card.length()) {
            return false;  // Invalid format, fail closed
        }
        
        // Extract suit (character after underscore)
        char suit = card[underscorePos + 1];
        
        // Validate suit is one of S, H, D, C
        if (suit != 'S' && suit != 'H' && suit != 'D' && suit != 'C') {
            return false;  // Invalid suit, fail closed
        }
        
        suitCount[suit]++;
    }
    
    // Process hand cards
    for (size_t i = 0; i < hand.size(); ++i) {
        const std::string& card = hand[i];
        
        // Validate card format
        if (card.length() < 3) {
            return false;  // Invalid card format, fail closed
        }
        
        size_t underscorePos = card.find('_');
        if (underscorePos == std::string::npos || underscorePos + 1 >= card.length()) {
            return false;  // Invalid format, fail closed
        }
        
        // Extract suit
        char suit = card[underscorePos + 1];
        
        // Validate suit
        if (suit != 'S' && suit != 'H' && suit != 'D' && suit != 'C') {
            return false;  // Invalid suit, fail closed
        }
        
        suitCount[suit]++;
    }
    
    // Check if any suit has 5 or more cards (flush)
    for (const auto& pair : suitCount) {
        if (pair.second >= 5) {
            return true;
        }
    }
    
    return false;
}

int main() {
    // Test case 1: Diamond flush
    std::vector<std::string> table1 = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    std::vector<std::string> hand1 = {"J_D", "3_D"};
    std::cout << "Test 1: " << (checkFlush(table1, hand1) ? "true" : "false") << std::endl;
    
    // Test case 2: Spade flush
    std::vector<std::string> table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    std::vector<std::string> hand2 = {"K_S", "Q_S"};
    std::cout << "Test 2: " << (checkFlush(table2, hand2) ? "true" : "false") << std::endl;
    
    // Test case 3: No flush
    std::vector<std::string> table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    std::vector<std::string> hand3 = {"3_S", "4_D"};
    std::cout << "Test 3: " << (checkFlush(table3, hand3) ? "true" : "false") << std::endl;
    
    // Test case 4: Heart flush
    std::vector<std::string> table4 = {"2_H", "5_H", "9_H", "K_H", "3_C"};
    std::vector<std::string> hand4 = {"A_H", "7_D"};
    std::cout << "Test 4: " << (checkFlush(table4, hand4) ? "true" : "false") << std::endl;
    
    // Test case 5: Club flush
    std::vector<std::string> table5 = {"6_C", "8_C", "9_C", "J_C", "2_D"};
    std::vector<std::string> hand5 = {"K_C", "A_S"};
    std::cout << "Test 5: " << (checkFlush(table5, hand5) ? "true" : "false") << std::endl;
    
    return 0;
}
