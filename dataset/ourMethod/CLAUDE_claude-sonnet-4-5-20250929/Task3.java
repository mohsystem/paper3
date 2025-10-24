
import java.util.HashSet;
import java.util.Set;

public class Task3 {
    public static boolean isPangram(String input) {
        // Validate input
        if (input == null) {
            return false;
        }
        
        // Normalize input to lowercase and track unique letters
        Set<Character> letters = new HashSet<>();
        
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            // Convert to lowercase and check if it's a letter\n            if (c >= 'A' && c <= 'Z') {\n                letters.add((char)(c + 32)); // Convert to lowercase\n            } else if (c >= 'a' && c <= 'z') {
                letters.add(c);
            }
        }
        
        // A pangram must contain all 26 letters
        return letters.size() == 26;
    }
    
    public static void main(String[] args) {
        // Test case 1: Classic pangram
        String test1 = "The quick brown fox jumps over the lazy dog";
        System.out.println("Test 1: \\"" + test1 + "\\" -> " + isPangram(test1));
        
        // Test case 2: Pangram with numbers and punctuation
        String test2 = "Pack my box with five dozen liquor jugs!!! 123";
        System.out.println("Test 2: \\"" + test2 + "\\" -> " + isPangram(test2));
        
        // Test case 3: Not a pangram
        String test3 = "This is not a pangram";
        System.out.println("Test 3: \\"" + test3 + "\\" -> " + isPangram(test3));
        
        // Test case 4: Empty string
        String test4 = "";
        System.out.println("Test 4: \\"" + test4 + "\\" -> " + isPangram(test4));
        
        // Test case 5: All letters present with special characters
        String test5 = "abcdefghijklmnopqrstuvwxyz!@#$%^&*()";
        System.out.println("Test 5: \\"" + test5 + "\\" -> " + isPangram(test5));
    }
}
