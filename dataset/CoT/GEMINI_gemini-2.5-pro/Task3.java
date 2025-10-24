import java.util.HashSet;
import java.util.Set;

public class Task3 {

    /**
     * Checks if a given string is a pangram.
     * A pangram is a sentence that contains every single letter of the alphabet at least once.
     * The check is case-insensitive and ignores numbers and punctuation.
     *
     * @param s The input string.
     * @return true if the string is a pangram, false otherwise.
     */
    public static boolean isPangram(String s) {
        if (s == null) {
            return false;
        }

        // A boolean array to mark the presence of each letter 'a' through 'z'.
        boolean[] alphabetSeen = new boolean[26];
        int uniqueLettersSeen = 0;

        // Convert the entire string to lower case to handle case-insensitivity.
        String lowerCaseString = s.toLowerCase();

        for (int i = 0; i < lowerCaseString.length(); i++) {
            char c = lowerCaseString.charAt(i);

            // Check if the character is a letter from 'a' to 'z'.
            if (c >= 'a' && c <= 'z') {
                int index = c - 'a';
                // If this letter has not been seen before, mark it and increment the count.
                if (!alphabetSeen[index]) {
                    alphabetSeen[index] = true;
                    uniqueLettersSeen++;
                }
            }
            
            // Optimization: if all 26 letters have been found, we can stop early.
            if (uniqueLettersSeen == 26) {
                return true;
            }
        }

        // The string is a pangram if we have seen all 26 letters.
        return uniqueLettersSeen == 26;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "The quick brown fox jumps over the lazy dog",
            "This is not a pangram",
            "Pack my box with five dozen liquor jugs.",
            "Cwm fjord bank glyphs vext quiz",
            "A an B b C c"
        };

        for (int i = 0; i < testCases.length; i++) {
            String testCase = testCases[i];
            boolean result = isPangram(testCase);
            System.out.println("Test Case " + (i + 1) + ": \"" + testCase + "\"");
            System.out.println("Is Pangram? " + result);
            System.out.println();
        }
    }
}