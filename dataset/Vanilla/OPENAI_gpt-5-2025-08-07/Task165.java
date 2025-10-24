import java.util.*;

public class Task165 {
    public static boolean checkFlush(String[] table, String[] hand) {
        int[] counts = new int[4]; // 0:S, 1:H, 2:D, 3:C
        for (String c : table) addSuitCount(c, counts);
        for (String c : hand) addSuitCount(c, counts);
        for (int v : counts) if (v >= 5) return true;
        return false;
    }

    private static void addSuitCount(String card, int[] counts) {
        char s = getSuit(card);
        switch (s) {
            case 'S': counts[0]++; break;
            case 'H': counts[1]++; break;
            case 'D': counts[2]++; break;
            case 'C': counts[3]++; break;
            default: break;
        }
    }

    private static char getSuit(String card) {
        int idx = card.lastIndexOf('_');
        return (idx >= 0 && idx + 1 < card.length()) ? card.charAt(idx + 1) : '?';
    }

    public static void main(String[] args) {
        String[] t1 = {"A_S", "J_H", "7_D", "8_D", "10_D"};
        String[] h1 = {"J_D", "3_D"};
        System.out.println(checkFlush(t1, h1)); // true

        String[] t2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
        String[] h2 = {"K_S", "Q_S"};
        System.out.println(checkFlush(t2, h2)); // true

        String[] t3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
        String[] h3 = {"3_S", "4_D"};
        System.out.println(checkFlush(t3, h3)); // false

        String[] t4 = {"2_H", "5_H", "7_H", "9_H", "K_H"};
        String[] h4 = {"A_S", "3_D"};
        System.out.println(checkFlush(t4, h4)); // true

        String[] t5 = {"2_C", "5_C", "7_C", "9_H", "K_D"};
        String[] h5 = {"A_C", "3_D"};
        System.out.println(checkFlush(t5, h5)); // false
    }
}