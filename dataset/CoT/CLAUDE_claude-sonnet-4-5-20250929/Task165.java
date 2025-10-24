
public class Task165 {
    public static boolean checkFlush(String[] table, String[] hand) {
        // Input validation
        if (table == null || hand == null || table.length != 5 || hand.length != 2) {
            return false;
        }
        
        // Count suits from all cards
        int[] suitCount = new int[4]; // S=0, H=1, D=2, C=3
        
        // Process table cards
        for (String card : table) {
            if (card == null || card.isEmpty()) {
                continue;
            }
            int suitIndex = getSuitIndex(card);
            if (suitIndex >= 0) {
                suitCount[suitIndex]++;
            }
        }
        
        // Process hand cards
        for (String card : hand) {
            if (card == null || card.isEmpty()) {
                continue;
            }
            int suitIndex = getSuitIndex(card);
            if (suitIndex >= 0) {
                suitCount[suitIndex]++;
            }
        }
        
        // Check if any suit has at least 5 cards (flush)
        for (int count : suitCount) {
            if (count >= 5) {
                return true;
            }
        }
        
        return false;
    }
    
    private static int getSuitIndex(String card) {
        // Safe extraction of suit
        int underscoreIndex = card.lastIndexOf('_');
        if (underscoreIndex < 0 || underscoreIndex >= card.length() - 1) {
            return -1;
        }
        
        char suit = card.charAt(underscoreIndex + 1);
        switch (suit) {
            case 'S': return 0;
            case 'H': return 1;
            case 'D': return 2;
            case 'C': return 3;
            default: return -1;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Diamond flush
        System.out.println(checkFlush(
            new String[]{"A_S", "J_H", "7_D", "8_D", "10_D"}, 
            new String[]{"J_D", "3_D"}
        )); // true
        
        // Test case 2: Spade flush
        System.out.println(checkFlush(
            new String[]{"10_S", "7_S", "9_H", "4_S", "3_S"}, 
            new String[]{"K_S", "Q_S"}
        )); // true
        
        // Test case 3: No flush
        System.out.println(checkFlush(
            new String[]{"3_S", "10_H", "10_D", "10_C", "10_S"}, 
            new String[]{"3_S", "4_D"}
        )); // false
        
        // Test case 4: Heart flush
        System.out.println(checkFlush(
            new String[]{"2_H", "5_H", "8_H", "K_H", "A_S"}, 
            new String[]{"Q_H", "J_H"}
        )); // true
        
        // Test case 5: Club flush
        System.out.println(checkFlush(
            new String[]{"9_C", "7_C", "4_C", "2_C", "K_H"}, 
            new String[]{"6_C", "3_D"}
        )); // true
    }
}
