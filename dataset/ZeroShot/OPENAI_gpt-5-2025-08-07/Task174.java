import java.util.*;

public class Task174 {
    public static String shortestPalindrome(String s) {
        if (s == null) return "";
        int n = s.length();
        if (n <= 1) return s;
        String rev = new StringBuilder(s).reverse().toString();
        String combined = s + "#" + rev;
        int[] lps = buildLPS(combined);
        int prefixLen = lps[combined.length() - 1];
        String suffix = s.substring(prefixLen);
        return new StringBuilder(suffix).reverse().toString() + s;
    }

    private static int[] buildLPS(String str) {
        int[] lps = new int[str.length()];
        int len = 0;
        for (int i = 1; i < str.length(); i++) {
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
            System.out.println(shortestPalindrome(t));
        }
    }
}