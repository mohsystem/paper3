import java.util.*;

public class Task30 {

    public static String longest(String s1, String s2) {
        boolean[] seen = new boolean[26];
        process(s1, seen);
        process(s2, seen);
        StringBuilder sb = new StringBuilder(26);
        for (int i = 0; i < 26; i++) {
            if (seen[i]) {
                sb.append((char) ('a' + i));
            }
        }
        return sb.toString();
    }

    private static void process(String s, boolean[] seen) {
        if (s == null) return;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c >= 'a' && c <= 'z') {
                seen[c - 'a'] = true;
            }
        }
    }

    public static void main(String[] args) {
        String[] a = {
            "xyaabbbccccdefww",
            "abcdefghijklmnopqrstuvwxyz",
            "",
            "abc123",
            "thequickbrownfox"
        };
        String[] b = {
            "xxxxyyyyabklmopq",
            "abcdefghijklmnopqrstuvwxyz",
            "",
            "def!ghi",
            "jumpsoverthelazydog"
        };
        for (int i = 0; i < 5; i++) {
            System.out.println(longest(a[i], b[i]));
        }
    }
}