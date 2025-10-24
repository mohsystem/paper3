import java.util.*;

public final class Task174 {

    public static String shortestPalindrome(String s) {
        if (s == null) {
            return "";
        }
        int n = s.length();
        if (n > 50000) {
            return "";
        }
        for (int i = 0; i < n; i++) {
            char ch = s.charAt(i);
            if (ch < 'a' || ch > 'z') {
                return "";
            }
        }
        if (n <= 1) {
            return s;
        }
        String rev = new StringBuilder(s).reverse().toString();
        String combined = s + "#" + rev;
        int[] lps = buildLPS(combined);
        int l = lps[combined.length() - 1];
        String add = rev.substring(0, n - l);
        return add + s;
    }

    private static int[] buildLPS(String str) {
        int m = str.length();
        int[] lps = new int[m];
        int len = 0;
        for (int i = 1; i < m; i++) {
            while (len > 0 && str.charAt(i) != str.charAt(len)) {
                len = lps[len - 1];
            }
            if (str.charAt(i) == str.charAt(len)) {
                len++;
            }
            lps[i] = len;
        }
        return lps;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
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