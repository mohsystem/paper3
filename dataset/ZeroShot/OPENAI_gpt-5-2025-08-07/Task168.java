import java.util.*;

public class Task168 {
    public static String tweakLetters(String s, int[] shifts) {
        if (s == null || shifts == null) {
            throw new IllegalArgumentException("Input cannot be null.");
        }
        if (s.length() != shifts.length) {
            throw new IllegalArgumentException("String length and shifts length must match.");
        }
        StringBuilder sb = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch >= 'a' && ch <= 'z') {
                int base = ch - 'a';
                int adj = shifts[i] % 26;
                if (adj < 0) adj += 26;
                int newIdx = (base + adj) % 26;
                sb.append((char) ('a' + newIdx));
            } else {
                sb.append(ch);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        try {
            System.out.println(tweakLetters("apple", new int[]{0, 1, -1, 0, -1})); // aqold
            System.out.println(tweakLetters("many", new int[]{0, 0, 0, -1}));      // manx
            System.out.println(tweakLetters("rhino", new int[]{1, 1, 1, 1, 1}));   // sijop
            System.out.println(tweakLetters("zzz", new int[]{1, 1, 1}));           // aaa
            System.out.println(tweakLetters("abc", new int[]{27, -28, 52}));       // bzc
        } catch (IllegalArgumentException e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}