
public class Task165 {
    public static boolean checkFlush(String[] table, String[] hand) {
        int[] suitCount = new int[4]; // S, H, D, C
        
        // Count suits from table
        for (String card : table) {
            String suit = card.split("_")[1];
            suitCount[getSuitIndex(suit)]++;
        }
        
        // Check each suit
        for (int i = 0; i < 4; i++) {
            if (suitCount[i] >= 3) {
                // Check if hand has cards of this suit
                String targetSuit = getSuitFromIndex(i);
                int handCount = 0;
                for (String card : hand) {
                    String suit = card.split("_")[1];
                    if (suit.equals(targetSuit)) {
                        handCount++;
                    }
                }
                // Need at least 5 total cards of same suit
                if (suitCount[i] + handCount >= 5) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    private static int getSuitIndex(String suit) {
        switch (suit) {
            case "S": return 0;
            case "H": return 1;
            case "D": return 2;
            case "C": return 3;
            default: return -1;
        }
    }
    
    private static String getSuitFromIndex(int index) {
        switch (index) {
            case 0: return "S";
            case 1: return "H";
            case 2: return "D";
            case 3: return "C";
            default: return "";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(checkFlush(
            new String[]{"A_S", "J_H", "7_D", "8_D", "10_D"}, 
            new String[]{"J_D", "3_D"}
        )); // true - diamond flush
        
        // Test case 2
        System.out.println(checkFlush(
            new String[]{"10_S", "7_S", "9_H", "4_S", "3_S"}, 
            new String[]{"K_S", "Q_S"}
        )); // true - spade flush
        
        // Test case 3
        System.out.println(checkFlush(
            new String[]{"3_S", "10_H", "10_D", "10_C", "10_S"}, 
            new String[]{"3_S", "4_D"}
        )); // false
        
        // Test case 4
        System.out.println(checkFlush(
            new String[]{"2_H", "5_H", "8_H", "K_C", "A_S"}, 
            new String[]{"9_H", "J_H"}
        )); // true - heart flush
        
        // Test case 5
        System.out.println(checkFlush(
            new String[]{"2_C", "3_C", "4_C", "5_H", "6_H"}, 
            new String[]{"7_H", "8_D"}
        )); // false
    }
}
