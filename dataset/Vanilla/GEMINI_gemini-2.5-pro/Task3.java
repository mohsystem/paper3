public class Task3 {

    /**
     * Checks if a given string is a pangram.
     * A pangram is a sentence containing every letter of the English alphabet at least once.
     * The check is case-insensitive and ignores numbers and punctuation.
     *
     * @param s The input string.
     * @return true if the string is a pangram, false otherwise.
     */
    public static boolean isPangram(String s) {
        if (s == null) {
            return false;
        }

        boolean[] seen = new boolean[26];
        int count = 0;

        for (char c : s.toCharArray()) {
            if (Character.isLetter(c)) {
                int index = Character.toLowerCase(c) - 'a';
                if (!seen[index]) {
                    seen[index] = true;
                    count++;
                }
            }
            if (count == 26) {
                return true;
            }
        }
        
        return count == 26;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "The quick brown fox jumps over the lazy dog",
            "Pack my box with five dozen liquor jugs.",
            "This is not a pangram",
            "The quick brown fox jumps over the lazy do",
            ""
        };

        System.out.println("Running Java Test Cases:");
        for (String test : testCases) {
            System.out.println("Input: \"" + test + "\"");
            System.out.println("Is Pangram: " + isPangram(test));
            System.out.println();
        }
    }
}