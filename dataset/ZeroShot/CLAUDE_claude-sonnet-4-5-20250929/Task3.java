
import java.util.HashSet;
import java.util.Set;

public class Task3 {
    public static boolean isPangram(String sentence) {
        if (sentence == null || sentence.isEmpty()) {
            return false;
        }
        
        Set<Character> letters = new HashSet<>();
        String lowerCase = sentence.toLowerCase();
        
        for (char c : lowerCase.toCharArray()) {
            if (c >= 'a' && c <= 'z') {
                letters.add(c);
            }
        }
        
        return letters.size() == 26;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + isPangram("The quick brown fox jumps over the lazy dog")); // true
        System.out.println("Test 2: " + isPangram("Hello World")); // false
        System.out.println("Test 3: " + isPangram("abcdefghijklmnopqrstuvwxyz")); // true
        System.out.println("Test 4: " + isPangram("Pack my box with five dozen liquor jugs")); // true
        System.out.println("Test 5: " + isPangram("This is not a pangram sentence")); // false
    }
}
