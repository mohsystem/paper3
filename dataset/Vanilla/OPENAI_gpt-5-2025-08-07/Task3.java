import java.util.*;

public class Task3 {
    public static boolean isPangram(String s) {
        boolean[] seen = new boolean[26];
        int count = 0;
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch >= 'A' && ch <= 'Z') ch = (char)(ch - 'A' + 'a');
            if (ch >= 'a' && ch <= 'z') {
                int idx = ch - 'a';
                if (!seen[idx]) {
                    seen[idx] = true;
                    count++;
                    if (count == 26) return true;
                }
            }
        }
        return count == 26;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "The quick brown fox jumps over the lazy dog",
            "Sphinx of black quartz, judge my vow",
            "Hello, World!",
            "",
            "Pack my box with five dozen liquor jugs."
        };
        for (String t : tests) {
            System.out.println(isPangram(t));
        }
    }
}