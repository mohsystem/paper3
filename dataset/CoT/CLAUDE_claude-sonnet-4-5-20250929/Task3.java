
import java.util.HashSet;
import java.util.Set;

public class Task3 {
    public static boolean isPangram(String text) {
        if (text == null || text.isEmpty()) {
            return false;
        }
        
        Set<Character> alphabetSet = new HashSet<>();
        String lowerText = text.toLowerCase();
        
        for (int i = 0; i < lowerText.length(); i++) {
            char c = lowerText.charAt(i);
            if (c >= 'a' && c <= 'z') {
                alphabetSet.add(c);
            }
        }
        
        return alphabetSet.size() == 26;
    }
    
    public static void main(String[] args) {
        // Test case 1: Classic pangram
        System.out.println("Test 1: " + isPangram("The quick brown fox jumps over the lazy dog"));
        
        // Test case 2: Pangram with numbers and punctuation
        System.out.println("Test 2: " + isPangram("Pack my box with five dozen liquor jugs!123"));
        
        // Test case 3: Not a pangram
        System.out.println("Test 3: " + isPangram("Hello World"));
        
        // Test case 4: Empty string
        System.out.println("Test 4: " + isPangram(""));
        
        // Test case 5: All letters with special characters
        System.out.println("Test 5: " + isPangram("abcdefghijklmnopqrstuvwxyz!!!"));
    }
}
