import java.util.*;

public class Lse7 {
    // Securely tries to parse a non-negative integer from a string. Returns true on success, false otherwise.
    // On success, the parsed value is placed into out[0].
    public static boolean tryParseNonNegativeInt(String s, int[] out) {
        if (out == null || out.length == 0) return false;
        if (s == null) return false;
        String t = s.trim();
        if (t.isEmpty()) return false;

        int i = 0;
        if (t.charAt(0) == '+') {
            if (t.length() == 1) return false;
            i = 1;
        } else if (t.charAt(0) == '-') {
            return false;
        }

        long val = 0;
        for (; i < t.length(); i++) {
            char c = t.charAt(i);
            if (c < '0' || c > '9') return false;
            val = val * 10 + (c - '0');
            if (val > Integer.MAX_VALUE) return false;
        }
        out[0] = (int) val;
        return true;
    }

    // Returns the argument at the index specified by indexStr, or null if invalid.
    public static String getArgAtIndex(String[] args, String indexStr) {
        if (args == null || indexStr == null) return null;
        int[] out = new int[1];
        if (!tryParseNonNegativeInt(indexStr, out)) return null;
        int idx = out[0];
        if (idx < 0 || idx >= args.length) return null;
        return args[idx];
    }

    // Prints the argument at the index specified by indexStr if valid.
    public static void printArgAtIndex(String[] args, String indexStr) {
        String v = getArgAtIndex(args, indexStr);
        if (v != null) {
            System.out.println(v);
        }
    }

    public static void main(String[] args) {
        // If run with CLI arguments, use the second argument as index, and print the argument at that index.
        if (args != null && args.length > 1) {
            printArgAtIndex(args, args[1]);
        }

        // 5 test cases
        String[][] tests = new String[][]{
            {"a0", "2", "x0", "x1", "x2"},          // prints x0
            {"first", "0", "z"},                    // prints first
            {"foo", "3", "aa", "bb", "cc", "dd"},   // prints dd
            {"unused", "-1", "val"},                // prints nothing (invalid)
            {"hello", "abc", "p", "q"}              // prints nothing (invalid)
        };

        for (String[] t : tests) {
            printArgAtIndex(t, t[1]);
        }
    }
}