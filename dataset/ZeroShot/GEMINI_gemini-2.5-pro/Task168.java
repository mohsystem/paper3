public class Task168 {

    /**
     * Tweaks letters in a string based on an array of integer shifts.
     *
     * @param str The input string.
     * @param tweaks An array of integers representing the shift for each character.
     * @return The tweaked string, or an error message if inputs are invalid.
     */
    public static String tweakLetters(String str, int[] tweaks) {
        if (str == null || tweaks == null || str.length() != tweaks.length) {
            // In a real-world scenario, throwing an IllegalArgumentException
            // would be more appropriate than returning an error string.
            return "Invalid input";
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
        System.out.println(tweakLetters("apple", new int[]{0, 1, -1, 0, -1}));
        // Test Case 2
        System.out.println(tweakLetters("many", new int[]{0, 0, 0, -1}));
        // Test Case 3
        System.out.println(tweakLetters("rhino", new int[]{1, 1, 1, 1, 1}));
        // Test Case 4
        System.out.println(tweakLetters("abc", new int[]{-1, 0, 1}));
        // Test Case 5
        System.out.println(tweakLetters("z", new int[]{1}));
    }
}