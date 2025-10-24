import java.util.Arrays;

class Task174 {

    /**
     * Computes the Longest Proper Prefix Suffix (LPS) array for KMP algorithm.
     *
     * @param pattern The string for which to compute the LPS array.
     * @return The LPS array.
     */
    private int[] computeLPSArray(String pattern) {
        int n = pattern.length();
        int[] lps = new int[n];
        int length = 0; // length of the previous longest prefix suffix
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
        int palindromicPrefixLength = lps[temp.length() - 1];

        String suffixToPrepend = s.substring(palindromicPrefixLength);
        String prefix = new StringBuilder(suffixToPrepend).reverse().toString();

        return prefix + s;
    }

    public static void main(String[] args) {
        Task174 solution = new Task174();

        // Test Case 1
        String s1 = "aacecaaa";
        System.out.println("Input: \"" + s1 + "\", Output: \"" + solution.shortestPalindrome(s1) + "\"");

        // Test Case 2
        String s2 = "abcd";
        System.out.println("Input: \"" + s2 + "\", Output: \"" + solution.shortestPalindrome(s2) + "\"");

        // Test Case 3
        String s3 = "abacaba";
        System.out.println("Input: \"" + s3 + "\", Output: \"" + solution.shortestPalindrome(s3) + "\"");

        // Test Case 4
        String s4 = "";
        System.out.println("Input: \"" + s4 + "\", Output: \"" + solution.shortestPalindrome(s4) + "\"");

        // Test Case 5
        String s5 = "abab";
        System.out.println("Input: \"" + s5 + "\", Output: \"" + solution.shortestPalindrome(s5) + "\"");
    }
}