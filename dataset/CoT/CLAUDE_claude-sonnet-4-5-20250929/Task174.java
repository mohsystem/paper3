
public class Task174 {
    public static String shortestPalindrome(String s) {
        // Input validation
        if (s == null || s.length() == 0) {
            return "";
        }
        
        if (s.length() > 50000) {
            throw new IllegalArgumentException("String length exceeds maximum allowed");
        }
        
        // Validate that string contains only lowercase letters
        for (char c : s.toCharArray()) {
            if (c < 'a' || c > 'z') {
                throw new IllegalArgumentException("String must contain only lowercase letters");
            }
        }
        
        // Use KMP algorithm to find longest palindromic prefix
        String rev = new StringBuilder(s).reverse().toString();
        String combined = s + "#" + rev;
        int[] lps = computeLPS(combined);
        
        // Find the part to prepend
        int palindromeLength = lps[combined.length() - 1];
        String toPrepend = rev.substring(0, s.length() - palindromeLength);
        
        return toPrepend + s;
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
        // Test cases
        System.out.println(shortestPalindrome("aacecaaa")); // "aaacecaaa"
        System.out.println(shortestPalindrome("abcd"));     // "dcbabcd"
        System.out.println(shortestPalindrome(""));         // ""
        System.out.println(shortestPalindrome("a"));        // "a"
        System.out.println(shortestPalindrome("abbacd"));   // "dcabbacd"
    }
}
