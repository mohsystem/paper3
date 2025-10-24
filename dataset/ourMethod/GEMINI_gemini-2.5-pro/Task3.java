public class Task3 {

    /**
     * Checks if a string is a pangram.
     * A pangram is a sentence containing every letter of the English alphabet at least once.
     * The check is case-insensitive and ignores numbers and punctuation.
     *
     * @param s The input string to check.
     * @return true if the string is a pangram, false otherwise.
     */
    public static boolean isPangram(String s) {
        if (s == null) {
            return false;
        }

        boolean[] seen = new boolean[26];
        int uniqueCount = 0;
        final int ALPHABET_SIZE = 26;

        for (char c : s.toLowerCase().toCharArray()) {
            if (c >= 'a' && c <= 'z') {
                int index = c - 'a';
                if (!seen[index]) {
                    seen[index] = true;
                    uniqueCount++;
                    if (uniqueCount == ALPHABET_SIZE) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "The quick brown fox jumps over the lazy dog",
            "This is not a pangram",
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
            "A quick brown fox jumps over the lazy cat",
            ""
        };

        boolean[] expectedResults = {
            true,
            false,
            true,
            false,
            false
        };

        for (int i = 0; i < testCases.length; i++) {
            boolean result = isPangram(testCases[i]);
            System.out.println("Test Case " + (i + 1) + ": \"" + testCases[i] + "\"");
            System.out.println("Is Pangram: " + result);
            System.out.println("Expected: " + expectedResults[i]);
            System.out.println("Result matches expected: " + (result == expectedResults[i]));
            System.out.println();
        }
    }
}