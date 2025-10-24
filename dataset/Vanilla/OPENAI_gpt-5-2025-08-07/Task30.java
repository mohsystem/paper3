import java.util.*;

public class Task30 {
    public static String longest(String s1, String s2) {
        boolean[] present = new boolean[26];
        for (int i = 0; i < s1.length(); i++) {
            char c = s1.charAt(i);
            if (c >= 'a' && c <= 'z') present[c - 'a'] = true;
        }
        for (int i = 0; i < s2.length(); i++) {
            char c = s2.charAt(i);
            if (c >= 'a' && c <= 'z') present[c - 'a'] = true;
        }
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 26; i++) {
            if (present[i]) sb.append((char) ('a' + i));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            {"xyaabbbccccdefww", "xxxxyyyyabklmopq"},
            {"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
            {"", ""},
            {"aaabbb", "bbbccc"},
            {"hello", "world"}
        };
        for (String[] t : tests) {
            System.out.println(longest(t[0], t[1]));
        }
    }
}