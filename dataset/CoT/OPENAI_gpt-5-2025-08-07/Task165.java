import java.util.*;

public class Task165 {
    // Step 1: Problem understanding and function definition
    // Determine if there is a flush (5+ cards of the same suit across table and hand).
    public static boolean checkFlush(String[] table, String[] hand) {
        int[] counts = new int[4]; // 0:S, 1:H, 2:D, 3:C
        addCards(counts, table);
        addCards(counts, hand);
        for (int c : counts) {
            if (c >= 5) return true;
        }
        return false;
    }

    // Step 2-4: Secure parsing and counting with input validation
    private static void addCards(int[] counts, String[] cards) {
        if (cards == null) return;
        for (String card : cards) {
            if (card == null) continue;
            int idx = card.indexOf('_');
            if (idx < 0 || idx + 1 >= card.length()) continue;
            char s = Character.toUpperCase(card.charAt(idx + 1));
            int si = suitIndex(s);
            if (si >= 0) counts[si]++;
        }
    }

    private static int suitIndex(char s) {
        switch (s) {
            case 'S': return 0;
            case 'H': return 1;
            case 'D': return 2;
            case 'C': return 3;
            default: return -1;
        }
    }

    // Step 5: Main with 5 test cases
    public static void main(String[] args) {
        String[][] tables = {
            {"A_S", "J_H", "7_D", "8_D", "10_D"},
            {"10_S", "7_S", "9_H", "4_S", "3_S"},
            {"3_S", "10_H", "10_D", "10_C", "10_S"},
            {"2_H", "5_H", "K_H", "9_C", "J_D"},
            {"a_s", "j_h", "7_d", "8_d", "10_d"}
        };
        String[][] hands = {
            {"J_D", "3_D"},
            {"K_S", "Q_S"},
            {"3_S", "4_D"},
            {"A_H", "3_H"},
            {"j_d", "3_c"}
        };

        for (int i = 0; i < tables.length; i++) {
            boolean res = checkFlush(tables[i], hands[i]);
            System.out.println("Test " + (i + 1) + ": " + res);
        }
    }
}