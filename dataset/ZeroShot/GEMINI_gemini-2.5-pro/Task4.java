import java.util.HashSet;
import java.util.Set;

public class Task4 {

    /**
     * Determines if a string is an isogram.
     * An isogram is a word that has no repeating letters. Case is ignored.
     * Assumes the input string contains only letters.
     *
     * @param str The string to check.
     * @return true if the string is an isogram, false otherwise.
     */
    public static boolean isIsogram(String str) {
        // An empty string is an isogram.
        if (str == null || str.isEmpty()) {
            return true;
        }

        // Using a boolean array is more efficient than a HashSet for a known,
        // small character set like the English alphabet.
        boolean[] seen = new boolean[26];
        String lowerCaseStr = str.toLowerCase();

        for (int i = 0; i < lowerCaseStr.length(); i++) {
            char c = lowerCaseStr.charAt(i);
            int index = c - 'a'; // Calculate index from 'a'

            // If we have seen this character before, it's not an isogram.
            if (seen[index]) {
                return false;
            }
            // Mark this character as seen.
            seen[index] = true;
        }

        // If the loop completes, no duplicates were found.
        return true;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "Dermatoglyphics",
            "aba",
            "moOse",
            "",
            "isogram"
        };

        System.out.println("Java Test Cases:");
        for (String test : testCases) {
            System.out.println("\"" + test + "\" --> " + isIsogram(test));
        }
    }
}