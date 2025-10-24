import java.util.HashSet;
import java.util.Set;

public class Task4 {

    /**
     * Determines if a string is an isogram (has no repeating letters, case-insensitive).
     *
     * @param str The string to check.
     * @return true if the string is an isogram, false otherwise.
     */
    public static boolean isIsogram(String str) {
        if (str == null) {
            return true; // Or handle as an error, but problem implies it's a valid case.
        }
        
        // Use a set to keep track of characters we've already seen.
        Set<Character> seen = new HashSet<>();
        
        // Convert the string to lower case to ignore case.
        String lowerStr = str.toLowerCase();
        
        // Iterate over each character in the string.
        for (char c : lowerStr.toCharArray()) {
            // If the set already contains the character, it's a repeat.
            if (seen.contains(c)) {
                return false;
            }
            // Otherwise, add the character to the set.
            seen.add(c);
        }
        
        // If we get through the whole string without finding repeats, it's an isogram.
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