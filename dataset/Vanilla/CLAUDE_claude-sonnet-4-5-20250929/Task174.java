
public class Task174 {
    public static String shortestPalindrome(String s) {
        if (s == null || s.length() == 0) {
            return s;
        }
        
        // Find the longest palindrome prefix
        String rev = new StringBuilder(s).reverse().toString();
        String combined = s + "#" + rev;
        
        // KMP failure function
        int[] lps = computeLPS(combined);
        int longestPalindromePrefix = lps[combined.length() - 1];
        
        // Add the remaining suffix in reverse to the front
        String suffix = s.substring(longestPalindromePrefix);
        return new StringBuilder(suffix).reverse().toString() + s;
    }
    
    private static int[] computeLPS(String s) {
        int n = s.length();
        int[] lps = new int[n];
        int len = 0;
        int i = 1;
        
        while (i < n) {
            if (s.charAt(i) == s.charAt(len)) {
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
        // Test case 1
        System.out.println("Test 1: " + shortestPalindrome("aacecaaa"));
        
        // Test case 2
        System.out.println("Test 2: " + shortestPalindrome("abcd"));
        
        // Test case 3
        System.out.println("Test 3: " + shortestPalindrome(""));
        
        // Test case 4
        System.out.println("Test 4: " + shortestPalindrome("a"));
        
        // Test case 5
        System.out.println("Test 5: " + shortestPalindrome("aba"));
    }
}
