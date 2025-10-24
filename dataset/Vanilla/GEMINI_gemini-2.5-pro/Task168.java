public class Task168 {

    /**
     * Tweaks letters by one forward (+1) or backwards (-1) according to an array.
     *
     * @param str The input string of lowercase letters.
     * @param tweaks An array of integers (0, 1, or -1) corresponding to each character.
     * @return The tweaked string.
     */
    public static String tweakLetters(String str, int[] tweaks) {
        if (str == null || tweaks == null || str.length() != tweaks.length) {
            return ""; // Or throw an exception for invalid input
        }
        StringBuilder result = new StringBuilder(str.length());
        for (int i = 0; i < str.length(); i++) {
            char c = str.charAt(i);
            int tweak = tweaks[i];
            char newChar = (char) (c + tweak);

            if (newChar > 'z') {
                newChar = 'a';
            } else if (newChar < 'a') {
                newChar = 'z';
            }
            result.append(newChar);
        }
        return result.toString();
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println(tweakLetters("apple", new int[]{0, 1, -1, 0, -1}));
        
        // Test Case 2
        System.out.println(tweakLetters("many", new int[]{0, 0, 0, -1}));
        
        // Test Case 3
        System.out.println(tweakLetters("rhino", new int[]{1, 1, 1, 1, 1}));
        
        // Test Case 4
        System.out.println(tweakLetters("zebra", new int[]{1, -1, 1, -1, 1}));
        
        // Test Case 5
        System.out.println(tweakLetters("abc", new int[]{-1, 0, 1}));
    }
}