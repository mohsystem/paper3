
public class Task168 {
    public static String tweakLetters(String str, int[] tweaks) {
        if (str == null || tweaks == null || str.length() != tweaks.length) {
            return str;
        }
        
        StringBuilder result = new StringBuilder();
        
        for (int i = 0; i < str.length(); i++) {
            char currentChar = str.charAt(i);
            int tweak = tweaks[i];
            
            // Apply the tweak to the character
            char newChar = (char) (currentChar + tweak);
            
            // Handle wrapping for lowercase letters
            if (currentChar >= 'a' && currentChar <= 'z') {
                if (newChar > 'z') {
                    newChar = (char) ('a' + (newChar - 'z' - 1));
                } else if (newChar < 'a') {
                    newChar = (char) ('z' - ('a' - newChar - 1));
                }
            }
            
            result.append(newChar);
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(tweakLetters("apple", new int[]{0, 1, -1, 0, -1}));
        // Expected: "aqold"
        
        // Test case 2
        System.out.println(tweakLetters("many", new int[]{0, 0, 0, -1}));
        // Expected: "manx"
        
        // Test case 3
        System.out.println(tweakLetters("rhino", new int[]{1, 1, 1, 1, 1}));
        // Expected: "sijop"
        
        // Test case 4
        System.out.println(tweakLetters("zebra", new int[]{1, 0, 0, 0, -1}));
        // Expected: "zebrq" (z+1 wraps to a, a-1 wraps to z)
        
        // Test case 5
        System.out.println(tweakLetters("hello", new int[]{0, 0, 0, 0, 0}));
        // Expected: "hello"
    }
}
