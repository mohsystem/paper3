import java.util.Arrays;

public class Task174 {

    /**
     * Finds the shortest palindrome by adding characters in front of the string.
     * This implementation uses the KMP algorithm's preprocessing step (LPS array)
     * to find the longest palindromic prefix of the string efficiently.
     *
     * @param s The input string.
     * @return The shortest palindrome string.
     */
    public String shortestPalindrome(String s) {
        if (s == null || s.length() <= 1) {
            return s;
        }

        String revS = new StringBuilder(s).reverse().toString();
        // Construct a temporary string: s + # + reverse(s)
        // The '#' is a separator to avoid matching across the two parts.
        String temp = s + "#" + revS;

        // Compute the LPS (Longest Proper Prefix which is also Suffix) array
        int[] lps = computeLPS(temp);

        // The last value in the LPS array gives the length of the longest
        // palindromic prefix of the original string 's'.
        int lpsLen = lps[temp.length() - 1];

        // The part of the string that is not part of the palindromic prefix
        // needs to be reversed and prepended.
        String suffix = s.substring(lpsLen);
        String prefix = new StringBuilder(suffix).reverse().toString();

        return prefix + s;
    }

    /**
     * Computes the Longest Proper Prefix Suffix (LPS) array for KMP algorithm.
     *
     * @param pattern The string for which to compute the LPS array.
     * @return The LPS array.
     */
    private int[] computeLPS(String pattern) {
        int n = pattern.length();
        int[] lps = new int[n]; // Initialized to zeros by default in Java
        int length = 0; // Length of the previous longest prefix suffix
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

    public static void main(String[] args) {
        Task174 sol = new Task174();
        
        String[] testCases = {"aacecaaa", "abcd", "a", "", "abacaba"};
        String[] expectedResults = {"aaacecaaa", "dcbabcd", "a", "", "abacaba"};

        for (int i = 0; i < testCases.length; i++) {
            String s = testCases[i];
            String expected = expectedResults[i];
            String result = sol.shortestPalindrome(s);
            System.out.println("Input: \"" + s + "\"");
            System.out.println("Output: \"" + result + "\"");
            System.out.println("Expected: \"" + expected + "\"");
            System.out.println(result.equals(expected) ? "Test PASSED" : "Test FAILED");
            System.out.println("--------------------");
        }
    }
}