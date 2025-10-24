
import java.util.*;

public class Task165 {
    public static boolean checkFlush(String[] table, String[] hand) {
        // Validate inputs
        if (table == null || hand == null) {
            return false;
        }
        if (table.length != 5 || hand.length != 2) {
            return false;
        }
        
        // Combine all cards
        List<String> allCards = new ArrayList<>();
        for (String card : table) {
            if (card == null || !isValidCard(card)) {
                return false;
            }
            allCards.add(card);
        }
        for (String card : hand) {
            if (card == null || !isValidCard(card)) {
                return false;
            }
            allCards.add(card);
        }
        
        // Count suits
        Map<Character, Integer> suitCount = new HashMap<>();
        for (String card : allCards) {
            char suit = card.charAt(card.length() - 1);
            suitCount.put(suit, suitCount.getOrDefault(suit, 0) + 1);
        }
        
        // Check if any suit has 5 or more cards
        for (int count : suitCount.values()) {
            if (count >= 5) {
                return true;
            }
        }
        
        return false;
    }
    
    private static boolean isValidCard(String card) {
        if (card.length() < 3) {
            return false;
        }
        if (card.indexOf('_') == -1) {
            return false;
        }
        
        String[] parts = card.split("_");
        if (parts.length != 2) {
            return false;
        }
        
        String rank = parts[0];
        String suit = parts[1];
        
        if (suit.length() != 1) {
            return false;
        }
        
        char suitChar = suit.charAt(0);
        if (suitChar != 'S' && suitChar != 'H' && suitChar != 'D' && suitChar != 'C') {
            return false;
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test case 1
        String[] table1 = {"A_S", "J_H", "7_D", "8_D", "10_D"};
        String[] hand1 = {"J_D", "3_D"};
        System.out.println("Test 1: " + checkFlush(table1, hand1)); // true
        
        // Test case 2
        String[] table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
        String[] hand2 = {"K_S", "Q_S"};
        System.out.println("Test 2: " + checkFlush(table2, hand2)); // true
        
        // Test case 3
        String[] table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
        String[] hand3 = {"3_S", "4_D"};
        System.out.println("Test 3: " + checkFlush(table3, hand3)); // false
        
        // Test case 4
        String[] table4 = {"2_H", "3_H", "4_H", "5_H", "6_C"};
        String[] hand4 = {"7_H", "8_C"};
        System.out.println("Test 4: " + checkFlush(table4, hand4)); // true
        
        // Test case 5
        String[] table5 = {"A_C", "K_D", "Q_H", "J_S", "10_C"};
        String[] hand5 = {"9_D", "8_H"};
        System.out.println("Test 5: " + checkFlush(table5, hand5)); // false
    }
}
