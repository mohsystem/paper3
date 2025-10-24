import java.nio.charset.StandardCharsets;

public class Task174 {

    /**
     * Computes the Longest Proper Prefix Suffix (LPS) array for KMP algorithm.
     * The LPS array stores for each position i, the length of the longest proper prefix of pattern[0...i]
     * which is also a suffix of pattern[0...i].
     *
     * @param pattern The string for which to compute the LPS array.
     * @return The LPS array.
     */
    private int[] computeLPSArray(String pattern) {
        int n = pattern.length();
        int[] lps = new int[n];
        int length = 0;
        int i = 1;

        while (i < n) {
            if (pattern.charAt(i) == pattern.charAt(length)) {
                length++;
                lps[i] = length;
                i++;
            } else {
                if (length != 0) {
                    length = lps[length - 1];
                } else {
                    lps[i] = 0;
                    i++;
                }
            }
        }
        return lps;
    }

    /**
     * Finds the shortest palindrome by adding characters to the front of the string.
     *
     * @param s The input string.
     * @return The shortest palindrome.
     */
    public String shortestPalindrome(String s) {
        if (s == null || s.length() <= 1) {
            return s;
        }

        String reversed_s = new StringBuilder(s).reverse().toString();
        String temp = s + "#" + reversed_s;
        
        int[] lps = computeLPSArray(temp);
        int lpsLength = lps[temp.length() - 1];

        String suffix = s.substring(lpsLength);
        String prefix = new StringBuilder(suffix).reverse().toString();

        return prefix + s;
    }

    public static void main(String[] args) {
        Task174 solver = new Task174();
        String[] testCases = {
            "aacecaaa",
            "abcd",
            "aba",
            "",
            "a"
        };
        String[] expectedResults = {
            "aaacecaaa",
            "dcbabcd",
            "aba",
            "",
            "a"
        };

        for (int i = 0; i < testCases.length; i++) {
            String s = testCases[i];
            String result = solver.shortestPalindrome(s);
            System.out.println("Input: \"" + s + "\"");
            System.out.println("Output: \"" + result + "\"");
            System.out.println("Expected: \"" + expectedResults[i] + "\"");
            System.out.println("Test " + (i + 1) + " " + (result.equals(expectedResults[i]) ? "Passed" : "Failed"));
            System.out.println();
        }
    }
}