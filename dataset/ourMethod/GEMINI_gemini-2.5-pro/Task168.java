import java.util.Arrays;

public class Task168 {

    /**
     * Tweaks letters in a string by one forward (+1) or backward (-1) according to an array.
     *
     * @param str The input string (assumed to be lowercase letters).
     * @param tweaks An array of integers (0, 1, or -1) of the same length as str.
     * @return A new string with the tweaked letters.
     * @throws IllegalArgumentException if inputs are null or have mismatched lengths.
     */
    public static String tweakLetters(String str, int[] tweaks) {
        if (str == null || tweaks == null) {
            throw new IllegalArgumentException("Input string and tweaks array cannot be null.");
        }
        if (str.length() != tweaks.length) {
            throw new IllegalArgumentException("Input string and tweaks array must have the same length.");
        }

        StringBuilder result = new StringBuilder(str.length());
        for (int i = 0; i < str.length(); i++) {
            char originalChar = str.charAt(i);
            int tweakValue = tweaks[i];
            char tweakedChar = (char) (originalChar + tweakValue);
            result.append(tweakedChar);
        }
        return result.toString();
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println("Test 1: 'apple', {0, 1, -1, 0, -1} -> " + tweakLetters("apple", new int[]{0, 1, -1, 0, -1}));
        
        // Test Case 2
        System.out.println("Test 2: 'many', {0, 0, 0, -1} -> " + tweakLetters("many", new int[]{0, 0, 0, -1}));
        
        // Test Case 3
        System.out.println("Test 3: 'rhino', {1, 1, 1, 1, 1} -> " + tweakLetters("rhino", new int[]{1, 1, 1, 1, 1}));
        
        // Test Case 4: Empty string
        System.out.println("Test 4: '', {} -> " + tweakLetters("", new int[]{}));
        
        // Test Case 5: All zeros
        System.out.println("Test 5: 'hello', {0, 0, 0, 0, 0} -> " + tweakLetters("hello", new int[]{0, 0, 0, 0, 0}));
    }
}