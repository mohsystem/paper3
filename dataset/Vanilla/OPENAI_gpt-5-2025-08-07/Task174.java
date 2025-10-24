import java.util.*;

public class Task174 {

    public static String shortestPalindrome(String s) {
        if (s == null || s.length() <= 1) return s == null ? "" : s;
        String rev = new StringBuilder(s).reverse().toString();
        String pattern = s + "#" + rev;
        int[] lps = buildLPS(pattern);
        int palLen = lps[pattern.length() - 1];
        String suffix = s.substring(palLen);
        String prefixToAdd = new StringBuilder(suffix).reverse().toString();
        return prefixToAdd + s;
    }

    private static int[] buildLPS(String str) {
        int n = str.length();
        int[] lps = new int[n];
        int len = 0;
        for (int i = 1; i < n; ) {
            if (str.charAt(i) == str.charAt(len)) {
                lps[i++] = ++len;
            } else if (len > 0) {
                len = lps[len - 1];
            } else {
                lps[i++] = 0;
            }
        }
        return lps;
    }

    public static void main(String[] args) {
        String[] tests = {
            "aacecaaa",
            "abcd",
            "",
            "a",
            "abbacd"
        };
        for (String t : tests) {
            System.out.println(shortestPalindrome(t));
        }
    }
}