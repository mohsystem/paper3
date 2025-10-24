
import java.util.Arrays;

public class Task168 {
    public static String tweakLetters(String str, int[] tweaks) {
        // Validate inputs
        if (str == null || tweaks == null) {
            throw new IllegalArgumentException("Input string and tweaks array cannot be null");
        }
        
        if (str.isEmpty()) {
            return str;
        }
        
        if (str.length() != tweaks.length) {
            throw new IllegalArgumentException("String length must match tweaks array length");
        }
        
        // Validate string contains only letters
        for (int i = 0; i < str.length(); i++) {
            char c = str.charAt(i);
            if (!Character.isLetter(c)) {
                throw new IllegalArgumentException("String must contain only letters");
            }
        }
        
        // Validate tweaks array contains only -1, 0, or 1
        for (int i = 0; i < tweaks.length; i++) {
            if (tweaks[i] < -1 || tweaks[i] > 1) {
                throw new IllegalArgumentException("Tweaks must be -1, 0, or 1");
            }
        }
        
        StringBuilder result = new StringBuilder(str.length());
        
        for (int i = 0; i < str.length(); i++) {
            char c = Character.toLowerCase(str.charAt(i));
            int tweak = tweaks[i];
            
            // Apply tweak with wrapping
            int charValue = c - 'a';
            charValue = (charValue + tweak + 26) % 26;
            char tweakedChar = (char) ('a' + charValue);
            
            result.append(tweakedChar);
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1
        String result1 = tweakLetters("apple", new int[]{0, 1, -1, 0, -1});
        System.out.println("Test 1: " + result1 + " (expected: aqold)");
        
        // Test case 2
        String result2 = tweakLetters("many", new int[]{0, 0, 0, -1});
        System.out.println("Test 2: " + result2 + " (expected: manx)");
        
        // Test case 3
        String result3 = tweakLetters("rhino", new int[]{1, 1, 1, 1, 1});
        System.out.println("Test 3: " + result3 + " (expected: sijop)");
        
        // Test case 4
        String result4 = tweakLetters("zebra", new int[]{1, 0, 0, 0, -1});
        System.out.println("Test 4: " + result4 + " (expected: aebrq)");
        
        // Test case 5
        String result5 = tweakLetters("code", new int[]{-1, -1, -1, -1});
        System.out.println("Test 5: " + result5 + " (expected: bncd)");
    }
}
