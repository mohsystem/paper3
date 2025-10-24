import java.util.HashMap;
import java.util.Map;

public class Task165 {

    /**
     * Determines if a flush exists given 5 table cards and 2 hand cards.
     * A flush is 5 or more cards of the same suit.
     *
     * @param table An array of 5 strings representing the cards on the table.
     * @param hand  An array of 2 strings representing the cards in hand.
     * @return true if a flush exists, false otherwise.
     */
    public static boolean checkFlush(String[] table, String[] hand) {
        // Securely validate input to prevent NullPointerException or ArrayOutOfBoundsException
        if (table == null || hand == null || table.length != 5 || hand.length != 2) {
            return false;
        }

        Map<Character, Integer> suitCounts = new HashMap<>();
        
        // Process table cards
        for (String card : table) {
            // Check for valid card string format to avoid exceptions
            if (card != null && card.length() >= 2 && card.indexOf('_') != -1) {
                char suit = card.charAt(card.length() - 1);
                suitCounts.put(suit, suitCounts.getOrDefault(suit, 0) + 1);
            }
        }

        // Process hand cards
        for (String card : hand) {
            if (card != null && card.length() >= 2 && card.indexOf('_') != -1) {
                char suit = card.charAt(card.length() - 1);
                suitCounts.put(suit, suitCounts.getOrDefault(suit, 0) + 1);
            }
        }

        // Check if any suit count is 5 or more
        for (int count : suitCounts.values()) {
            if (count >= 5) {
                return true;
            }
        }
        
        return false;
    }

    public static void main(String[] args) {
        // Test Case 1: Diamond flush
        String[] table1 = {"A_S", "J_H", "7_D", "8_D", "10_D"};
        String[] hand1 = {"J_D", "3_D"};
        System.out.println("Test Case 1: " + checkFlush(table1, hand1)); // Expected: true

        // Test Case 2: Spade flush (6 spades)
        String[] table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
        String[] hand2 = {"K_S", "Q_S"};
        System.out.println("Test Case 2: " + checkFlush(table2, hand2)); // Expected: true

        // Test Case 3: No flush
        String[] table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
        String[] hand3 = {"3_S", "4_D"};
        System.out.println("Test Case 3: " + checkFlush(table3, hand3)); // Expected: false

        // Test Case 4: No flush (4 Hearts, 3 Diamonds)
        String[] table4 = {"A_H", "2_H", "3_H", "4_H", "5_D"};
        String[] hand4 = {"6_D", "7_D"};
        System.out.println("Test Case 4: " + checkFlush(table4, hand4)); // Expected: false
        
        // Test Case 5: No flush (3 Clubs, 4 Spades)
        String[] table5 = {"A_C", "2_C", "3_C", "4_S", "5_S"};
        String[] hand5 = {"6_S", "7_S"};
        System.out.println("Test Case 5: " + checkFlush(table5, hand5)); // Expected: false
    }
}