import java.util.Arrays;

public class Task165 {
    public static boolean checkFlush(String[] table, String[] hand) {
        int[] counts = new int[4]; // 0:S, 1:H, 2:D, 3:C
        int[] tableCounts = new int[4];

        if (table == null || hand == null) return false;

        for (String card : table) {
            int idx = suitIndex(card);
            if (idx >= 0) {
                counts[idx]++;
                tableCounts[idx]++;
            }
        }

        // Optimization based on hint: need at least 3 of a suit on table
        boolean possible = false;
        for (int c : tableCounts) {
            if (c >= 3) { possible = true; break; }
        }
        if (!possible) return false;

        for (String card : hand) {
            int idx = suitIndex(card);
            if (idx >= 0) counts[idx]++;
        }

        for (int c : counts) {
            if (c >= 5) return true;
        }
        return false;
    }

    private static int suitIndex(String card) {
        if (card == null) return -1;
        String s = card.trim();
        int idx = s.lastIndexOf('_');
        if (idx < 0 || idx + 1 >= s.length()) return -1;
        char ch = Character.toUpperCase(s.charAt(idx + 1));
        switch (ch) {
            case 'S': return 0;
            case 'H': return 1;
            case 'D': return 2;
            case 'C': return 3;
            default: return -1;
        }
    }

    // 5 test cases
    public static void main(String[] args) {
        String[][] tables = {
            {"A_S", "J_H", "7_D", "8_D", "10_D"},
            {"10_S", "7_S", "9_H", "4_S", "3_S"},
            {"3_S", "10_H", "10_D", "10_C", "10_S"},
            {"2_H", "5_C", "9_D", "K_H", "Q_H"},
            {"A_C", "2_D", "3_H", "4_S", "5_C"}
        };
        String[][] hands = {
            {"J_D", "3_D"},
            {"K_S", "Q_S"},
            {"3_S", "4_D"},
            {"A_H", "3_H"},
            {"6_D", "7_H"}
        };

        for (int i = 0; i < tables.length; i++) {
            System.out.println("Test " + (i + 1) + ": " + checkFlush(tables[i], hands[i]));
        }
    }
}