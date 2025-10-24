import java.util.HashMap;
import java.util.Map;

public class Task165 {

    /**
     * Determines if a 5-card flush can be made from 5 table cards and 2 hand cards.
     *
     * @param tableCards An array of 5 strings representing cards on the table.
     * @param handCards  An array of 2 strings representing cards in hand.
     * @return True if a flush exists, false otherwise.
     */
    public static boolean checkFlush(String[] tableCards, String[] handCards) {
        Map<Character, Integer> suitCounts = new HashMap<>();

        // Process table cards
        for (String card : tableCards) {
            char suit = card.charAt(card.length() - 1);
            suitCounts.put(suit, suitCounts.getOrDefault(suit, 0) + 1);
        }

        // Process hand cards
        for (String card : handCards) {
            char suit = card.charAt(card.length() - 1);
            suitCounts.put(suit, suitCounts.getOrDefault(suit, 0) + 1);
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
        System.out.println(checkFlush(table1, hand1));

        // Test Case 2
        String[] table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
        String[] hand2 = {"K_S", "Q_S"};
        System.out.println(checkFlush(table2, hand2));

        // Test Case 3
        String[] table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
        String[] hand3 = {"3_S", "4_D"};
        System.out.println(checkFlush(table3, hand3));

        // Test Case 4
        String[] table4 = {"A_H", "K_H", "Q_H", "J_H", "10_H"};
        String[] hand4 = {"3_S", "4_D"};
        System.out.println(checkFlush(table4, hand4));

        // Test Case 5
        String[] table5 = {"A_S", "K_S", "Q_S", "J_S", "2_H"};
        String[] hand5 = {"3_D", "4_C"};
        System.out.println(checkFlush(table5, hand5));
    }
}