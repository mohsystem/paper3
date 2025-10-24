// Step 1-5: Secure, efficient implementation using KMP to find longest palindromic prefix.
import java.util.*;

public class Task174 {

    // Computes the shortest palindrome by adding characters in front of the input string.
    public static String shortestPalindrome(String s) {
        if (s == null) return "";
        int n = s.length();
        if (n <= 1) return s;

        String rev = new StringBuilder(s).reverse().toString();
        String combined = s + "#" + rev; // Delimiter to avoid overlap
        int[] pi = prefixFunction(combined);
        int L = pi[combined.length() - 1];
        return new StringBuilder(s.substring(L)).reverse().append(s).toString();
    }

    // KMP prefix function
    private static int[] prefixFunction(String str) {
        int n = str.length();
        int[] pi = new int[n];
        for (int i = 1; i < n; i++) {
            int j = pi[i - 1];
            while (j > 0 && str.charAt(i) != str.charAt(j)) {
                j = pi[j - 1];
            }
            if (str.charAt(i) == str.charAt(j)) {
                j++;
            }
            pi[i] = j;
        }
        return pi;
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String[] tests = {
            "aacecaaa",
            "abcd",
            "",
            "a",
            "aaab"
        };
        for (String t : tests) {
            String res = shortestPalindrome(t);
            System.out.println("Input: \"" + t + "\" -> Output: \"" + res + "\"");
        }
    }
}