import java.util.HashMap;
import java.util.Map;

public class Task165 {

    /**
     * Checks if a flush can be formed from table and hand cards.
     * @param tableCards 5 cards on the table.
     * @param handCards 2 cards in hand.
     * @return true if a flush exists, false otherwise.
     */
    public static boolean checkFlush(String[] tableCards, String[] handCards) {
        if (tableCards == null || tableCards.length != 5 || handCards == null || handCards.length != 2) {
            return false;
        }

        Map<Character, Integer> suitCounts = new HashMap<>();
        suitCounts.put('S', 0);
        suitCounts.put('H', 0);
        suitCounts.put('D', 0);
        suitCounts.put('C', 0);

        for (String card : tableCards) {
            if (!isValidCard(card)) {
                return false;
            }
            char suit = card.charAt(card.length() - 1);
            suitCounts.put(suit, suitCounts.get(suit) + 1);
        }

        for (String card : handCards) {
            if (!isValidCard(card)) {
                return false;
            }
            char suit = card.charAt(card.length() - 1);
            suitCounts.put(suit, suitCounts.get(suit) + 1);
        }

        for (int count : suitCounts.values()) {
            if (count >= 5) {
                return true;
            }
        }

        return false;
    }

    /**
     * Validates the card string format.
     * @param card The card string to validate.
     * @return true if the card is valid, false otherwise.
     */
    private static boolean isValidCard(String card) {
        if (card == null || card.length() < 3 || !card.contains("_")) {
            return false;
        }
        char suit = card.charAt(card.length() - 1);
        return suit == 'S' || suit == 'H' || suit == 'D' || suit == 'C';
    }

    public static void main(String[] args) {
        // Test case 1
        String[] table1 = {"A_S", "J_H", "7_D", "8_D", "10_D"};
        String[] hand1 = {"J_D", "3_D"};
        System.out.println("Test 1: " + checkFlush(table1, hand1)); // Expected: true

        // Test case 2
        String[] table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
        String[] hand2 = {"K_S", "Q_S"};
        System.out.println("Test 2: " + checkFlush(table2, hand2)); // Expected: true

        // Test case 3
        String[] table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
        String[] hand3 = {"3_S", "4_D"};
        System.out.println("Test 3: " + checkFlush(table3, hand3)); // Expected: false

        // Test case 4
        String[] table4 = {"A_C", "K_C", "Q_C", "J_C", "10_C"};
        String[] hand4 = {"2_H", "3_D"};
        System.out.println("Test 4: " + checkFlush(table4, hand4)); // Expected: true

        // Test case 5
        String[] table5 = {"A_H", "A_S", "A_D", "A_C", "K_H"};
        String[] hand5 = {"K_S", "K_D"};
        System.out.println("Test 5: " + checkFlush(table5, hand5)); // Expected: false
    }
}