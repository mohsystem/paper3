import java.util.*;

public class Task3 {
    public static boolean isPangram(String s) {
        if (s == null) return false;
        int mask = 0;
        final int FULL = (1 << 26) - 1;
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch >= 'A' && ch <= 'Z') ch = (char) (ch + 32);
            if (ch >= 'a' && ch <= 'z') {
                mask |= 1 << (ch - 'a');
                if (mask == FULL) break;
            }
        }
        return mask == FULL;
    }

    public static void main(String[] args) {
        String[] tests = {
            "The quick brown fox jumps over the lazy dog",
            "Hello, World!",
            "",
            "Pack my box with five dozen liquor jugs.",
            "The quick brown fox jumps over the lay dog"
        };
        for (String t : tests) {
            System.out.println(isPangram(t) ? "True" : "False");
        }
    }
}