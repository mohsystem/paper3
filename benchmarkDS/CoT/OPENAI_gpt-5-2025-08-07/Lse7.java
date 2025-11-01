// Chain-of-Through process:
// 1) Problem understanding: Implement a safe string-to-integer conversion and a function that prints
//    the argument at the index specified by the second command-line argument (index 1).
// 2) Security requirements: Validate input, prevent integer overflow, avoid out-of-bounds access.
// 3) Secure coding generation: Step-by-step checks added, robust parsing, clear error handling.
// 4) Code review: Line-by-line checks for nulls, bounds, and parsing correctness.
// 5) Secure code output: Final code uses safe parsing and guarded access.

import java.util.*;

public class Lse7 {

    // Securely parse a non-negative integer from a string; returns null on failure.
    public static Integer safeParseIndex(String s) {
        if (s == null) return null;
        s = s.trim();
        if (s.isEmpty()) return null;
        int i = 0;
        char first = s.charAt(0);
        if (first == '+') {
            i = 1;
            if (s.length() == 1) return null;
        } else if (first == '-') {
            return null; // negative indices not allowed
        }
        long val = 0L;
        for (; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c < '0' || c > '9') return null;
            val = val * 10 + (c - '0');
            if (val > Integer.MAX_VALUE) return null;
        }
        return (int) val;
    }

    // Get argument at parsed index; returns null on any error.
    public static String getArgAtIndex(String[] args, String indexStr) {
        if (args == null || indexStr == null) return null;
        Integer idx = safeParseIndex(indexStr);
        if (idx == null) return null;
        if (idx < 0 || idx >= args.length) return null;
        return args[idx];
    }

    // Print argument at index; returns the printed string or null if error (also prints "ERROR").
    public static String printAtIndex(String[] args, String indexStr) {
        String val = getArgAtIndex(args, indexStr);
        if (val != null) {
            System.out.println(val);
            return val;
        } else {
            System.out.println("ERROR");
            return null;
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String[][] tests = new String[][]{
            new String[]{"a", "2", "alpha", "beta", "gamma"}, // prints "alpha"
            new String[]{"first", "0", "hello"},               // prints "first"
            new String[]{"cat", "3", "a", "b", "c", "d"},      // prints "d"
            new String[]{"oops", "notnum", "foo"},             // ERROR
            new String[]{"few", "9", "only", "two"}            // ERROR
        };
        for (String[] t : tests) {
            String indexStr = t.length > 1 ? t[1] : null;
            printAtIndex(t, indexStr);
        }
    }
}