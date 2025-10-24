
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;

public class Task174 {
    public static String shortestPalindrome(String s) {
        // Input validation
        if (s == null) {
            throw new IllegalArgumentException("Input string cannot be null");
        }
        
        // Validate length constraint
        if (s.length() > 50000) {
            throw new IllegalArgumentException("Input string length exceeds maximum allowed (50000)");
        }
        
        // Validate that string contains only lowercase English letters
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c < 'a' || c > 'z') {
                throw new IllegalArgumentException("Input string must contain only lowercase English letters");
            }
        }
        
        if (s.isEmpty()) {
            return "";
        }
        
        // Find the longest palindrome prefix
        String rev = new StringBuilder(s).reverse().toString();
        String combined = s + "#" + rev;
        
        int[] lps = computeLPS(combined);
        int longestPalindromePrefix = lps[combined.length() - 1];
        
        // Add the suffix in reverse to the front
        String suffix = s.substring(longestPalindromePrefix);
        String prefix = new StringBuilder(suffix).reverse().toString();
        
        return prefix + s;
    }
    
    private static int[] computeLPS(String pattern) {
        int n = pattern.length();
        int[] lps = new int[n];
        int len = 0;
        int i = 1;
        
        while (i < n) {
            if (pattern.charAt(i) == pattern.charAt(len)) {
                len++;
                lps[i] = len;
                i++;
            } else {
                if (len != 0) {
                    len = lps[len - 1];
                } else {
                    lps[i] = 0;
                    i++;
                }
            }
        }
        
        return lps;
    }
    
    public static void main(String[] args) {
        List<String> testCases = Arrays.asList("aacecaaa", "abcd", "", "a", "racecar");
        
        for (String testCase : testCases) {
            System.out.println("Input: \\"" + testCase + "\\"");
            System.out.println("Output: \\"" + shortestPalindrome(testCase) + "\\"");
            System.out.println();
        }
    }
}
