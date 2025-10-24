
public class Task168 {
    public static String tweakLetters(String str, int[] tweaks) {
        // Input validation
        if (str == null || tweaks == null) {
            return "";
        }
        
        if (str.isEmpty()) {
            return "";
        }
        
        // Ensure arrays have matching lengths
        int length = Math.min(str.length(), tweaks.length);
        StringBuilder result = new StringBuilder();
        
        for (int i = 0; i < length; i++) {
            char c = str.charAt(i);
            int tweak = tweaks[i];
            
            // Validate tweak value is within safe range
            if (tweak < -25 || tweak > 25) {
                result.append(c);
                continue;
            }
            
            // Only process lowercase letters
            if (c >= 'a' && c <= 'z') {
                // Calculate new character with wrapping
                int offset = c - 'a';
                int newOffset = (offset + tweak) % 26;
                // Handle negative modulo
                if (newOffset < 0) {
                    newOffset += 26;
                }
                result.append((char)('a' + newOffset));
            } else {
                // Non-lowercase letters remain unchanged
                result.append(c);
            }
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
        System.out.println(tweakLetters("zebra", new int[]{1, -1, 0, 1, -1}));
        // Expected: "aabrz"
        
        // Test case 5
        System.out.println(tweakLetters("code", new int[]{-1, 1, -1, 1}));
        // Expected: "bpcf"
    }
}
