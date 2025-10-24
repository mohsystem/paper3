import java.util.HashMap;
import java.util.Map;

public class Task165 {

    /**
     * Determines if a flush is possible given 5 table cards and 2 hand cards.
     * A flush is 5 or more cards of the same suit.
     *
     * @param table An array of 5 strings representing cards on the table.
     * @param hand  An array of 2 strings representing cards in hand.
     * @return true if a flush exists, false otherwise.
     */
    public static boolean checkFlush(String[] table, String[] hand) {
        Map<Character, Integer> suitCounts = new HashMap<>();
        suitCounts.put('S', 0);
        suitCounts.put('H', 0);
        suitCounts.put('D', 0);
        suitCounts.put('C', 0);

        // Process table cards
        for (String card : table) {
            if (card != null && !card.isEmpty()) {
                char suit = card.charAt(card.length() - 1);
                if (suitCounts.containsKey(suit)) {
                    suitCounts.put(suit, suitCounts.get(suit) + 1);
                }
            }
        }

        // Process hand cards
        for (String card : hand) {
            if (card != null && !card.isEmpty()) {
                char suit = card.charAt(card.length() - 1);
                if (suitCounts.containsKey(suit)) {
                    suitCounts.put(suit, suitCounts.get(suit) + 1);
                }
            }
        }

        // Check for a flush (5 or more cards of the same suit)
        for (int count : suitCounts.values()) {
            if (count >= 5) {
                return true;
            }
        }

        return false;
    }

    public static void main(String[] args) {
        // Test Case 1
        String[] table1 = {"A_S", "J_H", "7_D", "8_D", "10_D"};
        String[] hand1 = {"J_D", "3_D"};
        System.out.println("Test Case 1: " + checkFlush(table1, hand1)); // Expected: true

        // Test Case 2
        String[] table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
        String[] hand2 = {"K_S", "Q_S"};
        System.out.println("Test Case 2: " + checkFlush(table2, hand2)); // Expected: true

        // Test Case 3
        String[] table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
        String[] hand3 = {"3_S", "4_D"};
        System.out.println("Test Case 3: " + checkFlush(table3, hand3)); // Expected: false

        // Test Case 4
        String[] table4 = {"A_H", "A_S", "A_D", "A_C", "2_H"};
        String[] hand4 = {"3_H", "4_H"};
        System.out.println("Test Case 4: " + checkFlush(table4, hand4)); // Expected: false (4 Hearts)

        // Test Case 5
        String[] table5 = {"K_C", "Q_C", "J_C", "3_S", "4_H"};
        String[] hand5 = {"10_C", "9_C"};
        System.out.println("Test Case 5: " + checkFlush(table5, hand5)); // Expected: true (5 Clubs)
    }
}