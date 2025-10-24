public class Task4 {

    /**
     * Checks if a string is an isogram (has no repeating letters, case-insensitive).
     *
     * @param str The string to check. Assumed to contain only letters.
     * @return true if the string is an isogram, false otherwise.
     */
    public static boolean isIsogram(String str) {
        if (str == null) {
            return false; // Or throw an exception, depending on desired behavior for invalid input.
        }

        // A boolean array is more efficient than a HashSet for a fixed, small character set like the alphabet.
        boolean[] seen = new boolean[26];
        String lowerCaseStr = str.toLowerCase();

        for (int i = 0; i < lowerCaseStr.length(); i++) {
            char c = lowerCaseStr.charAt(i);
            // Assuming input contains only letters as per the prompt.
            // A more robust solution would check if c is in the range 'a' to 'z'.
            int index = c - 'a';
            if (seen[index]) {
                return false;
            }
            seen[index] = true;
        }

        return true;
    }

    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "Dermatoglyphics", // true
            "aba",             // false
            "moOse",           // false
            "isogram",         // true
            ""                 // true
        };

        for (String testCase : testCases) {
            System.out.println("\"" + testCase + "\" --> " + isIsogram(testCase));
        }
    }
}