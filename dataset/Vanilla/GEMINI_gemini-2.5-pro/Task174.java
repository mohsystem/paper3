import java.util.Arrays;

public class Task174 {

    /**
     * Computes the Longest Proper Prefix which is also a Suffix (LPS) array for KMP algorithm.
     * @param pattern The string for which to compute the LPS array.
     * @return The LPS array.
     */
    private int[] computeLPSArray(String pattern) {
        int n = pattern.length();
        int[] lps = new int[n];
        int length = 0; // Length of the previous longest prefix suffix
        int i = 1;
        lps[0] = 0; // lps[0] is always 0

        // The loop calculates lps[i] for i = 1 to n-1
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
     * Finds the shortest palindrome by adding characters in front of the given string.
     * @param s The input string.
     * @return The shortest palindrome.
     */
    public String shortestPalindrome(String s) {
        if (s == null || s.length() <= 1) {
            return s;
        }

        int n = s.length();
        String rev_s = new StringBuilder(s).reverse().toString();
        // Create a temporary string for KMP: s + special_char + reversed_s
        String temp = s + "#" + rev_s;
        
        // Compute the LPS array for the temporary string
        int[] lps = computeLPSArray(temp);
        
        // The last value of lps array gives the length of the longest palindromic prefix of s
        int l = lps[temp.length() - 1];
        
        // The prefix to add is the non-palindromic part of the reversed string
        String prefix = rev_s.substring(0, n - l);
        
        return prefix + s;
    }
    
    public static void main(String[] args) {
        Task174 solution = new Task174();

        // Test Case 1
        String s1 = "aacecaaa";
        System.out.println("Input: " + s1);
        System.out.println("Output: " + solution.shortestPalindrome(s1)); // Expected: "aaacecaaa"
        System.out.println();

        // Test Case 2
        String s2 = "abcd";
        System.out.println("Input: " + s2);
        System.out.println("Output: " + solution.shortestPalindrome(s2)); // Expected: "dcbabcd"
        System.out.println();

        // Test Case 3
        String s3 = "ababa";
        System.out.println("Input: " + s3);
        System.out.println("Output: " + solution.shortestPalindrome(s3)); // Expected: "ababa"
        System.out.println();

        // Test Case 4
        String s4 = "race";
        System.out.println("Input: " + s4);
        System.out.println("Output: " + solution.shortestPalindrome(s4)); // Expected: "ecarace"
        System.out.println();
        
        // Test Case 5
        String s5 = "a";
        System.out.println("Input: " + s5);
        System.out.println("Output: " + solution.shortestPalindrome(s5)); // Expected: "a"
        System.out.println();
    }
}