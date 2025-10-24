
public class Task174 {
    public static String shortestPalindrome(String s) {
        if (s == null || s.length() == 0) {
            return s;
        }
        
        // Find the longest palindrome prefix using KMP algorithm
        String rev = new StringBuilder(s).reverse().toString();
        String combined = s + "#" + rev;
        
        int[] lps = computeLPS(combined);
        int longestPalindromePrefix = lps[combined.length() - 1];
        
        // Add the reversed suffix to the front
        String suffix = s.substring(longestPalindromePrefix);
        String prefixToAdd = new StringBuilder(suffix).reverse().toString();
        
        return prefixToAdd + s;
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
        String test1 = "aacecaaa";
        System.out.println("Input: " + test1);
        System.out.println("Output: " + shortestPalindrome(test1));
        System.out.println();
        
        // Test case 2
        String test2 = "abcd";
        System.out.println("Input: " + test2);
        System.out.println("Output: " + shortestPalindrome(test2));
        System.out.println();
        
        // Test case 3
        String test3 = "";
        System.out.println("Input: \\"" + test3 + "\\"");
        System.out.println("Output: \\"" + shortestPalindrome(test3) + "\\"");
        System.out.println();
        
        // Test case 4
        String test4 = "a";
        System.out.println("Input: " + test4);
        System.out.println("Output: " + shortestPalindrome(test4));
        System.out.println();
        
        // Test case 5
        String test5 = "abbacd";
        System.out.println("Input: " + test5);
        System.out.println("Output: " + shortestPalindrome(test5));
        System.out.println();
    }
}
