import java.util.*;

public class Task165 {

    public static boolean checkFlush(String[] table, String[] hand) {
        if (table == null || hand == null || table.length != 5 || hand.length != 2) {
            return false;
        }

        int[] tableCounts = new int[4]; // 0:S,1:H,2:D,3:C
        Arrays.fill(tableCounts, 0);

        for (String card : table) {
            int idx = suitIndex(card);
            if (idx < 0) return false;
            tableCounts[idx]++;
        }

        // Early exit: if no suit has at least 3 on the table, flush is impossible
        boolean possible = false;
        for (int c : tableCounts) {
            if (c >= 3) {
                possible = true;
                break;
            }
        }
        if (!possible) return false;

        int[] totalCounts = Arrays.copyOf(tableCounts, tableCounts.length);
        for (String card : hand) {
            int idx = suitIndex(card);
            if (idx < 0) return false;
            totalCounts[idx]++;
        }

        for (int c : totalCounts) {
            if (c >= 5) return true;
        }
        return false;
    }

    private static int suitIndex(String card) {
        if (card == null) return -1;
        int first = card.indexOf('_');
        int last = card.lastIndexOf('_');
        if (first <= 0 || first != last || first == card.length() - 1) {
            return -1;
        }
        String suitPart = card.substring(first + 1);
        if (suitPart.length() != 1) return -1;
        char s = suitPart.charAt(0);
        switch (s) {
            case 'S': return 0;
            case 'H': return 1;
            case 'D': return 2;
            case 'C': return 3;
            default: return -1;
        }
    }

    public static void main(String[] args) {
        // Test case 1: diamond flush
        System.out.println(checkFlush(
                new String[]{"A_S", "J_H", "7_D", "8_D", "10_D"},
                new String[]{"J_D", "3_D"})); // true

        // Test case 2: spade flush
        System.out.println(checkFlush(
                new String[]{"10_S", "7_S", "9_H", "4_S", "3_S"},
                new String[]{"K_S", "Q_S"})); // true

        // Test case 3: no flush
        System.out.println(checkFlush(
                new String[]{"3_S", "10_H", "10_D", "10_C", "10_S"},
                new String[]{"3_S", "4_D"})); // false

        // Test case 4: clubs flush using 3 on table + 2 in hand
        System.out.println(checkFlush(
                new String[]{"2_C", "5_C", "9_C", "K_H", "A_D"},
                new String[]{"3_C", "7_C"})); // true

        // Test case 5: four hearts on table, none in hand -> no flush
        System.out.println(checkFlush(
                new String[]{"2_H", "4_H", "6_H", "8_H", "J_D"},
                new String[]{"Q_S", "K_C"})); // false
    }
}