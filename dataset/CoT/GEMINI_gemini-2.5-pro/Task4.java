import java.util.Arrays;

public class Task4 {

    /**
     * Determines whether a string is an isogram (has no repeating letters, ignoring case).
     *
     * @param str The input string, assumed to contain only letters.
     * @return true if the string is an isogram, false otherwise.
     */
    public static boolean isIsogram(String str) {
        // An empty string is considered an isogram. A null check is good practice.
        if (str == null || str.isEmpty()) {
            return true;
        }

        // Convert the string to a single case to make comparisons case-insensitive.
        String lowerCaseStr = str.toLowerCase();

        // Use a boolean array to track seen characters.
        // The size is 26 for the English alphabet (a-z).
        boolean[] seen = new boolean[26];
        Arrays.fill(seen, false);

        // Iterate through each character of the string.
        for (int i = 0; i < lowerCaseStr.length(); i++) {
            char c = lowerCaseStr.charAt(i);
            int index = c - 'a';

            // If the character has been seen before, it's not an isogram.
            if (seen[index]) {
                return false;
            }

            // Mark the character as seen.
            seen[index] = true;
        }

        // If the loop completes, no repeating characters were found.
        return true;
    }

    public static void main(String[] args) {
        // Test cases
        String[] testCases = {"Dermatoglyphics", "aba", "moOse", "isIsogram", ""};
        boolean[] expectedResults = {true, false, false, false, true};

        for (int i = 0; i < testCases.length; i++) {
            boolean result = isIsogram(testCases[i]);
            System.out.println("Input: \"" + testCases[i] + "\" -> Output: " + result + " | Expected: " + expectedResults[i]);
        }
    }
}