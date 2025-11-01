import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse7 {
    // Converts a non-negative decimal string to int with overflow and format checks.
    // Returns null if invalid.
    public static Integer stringToInt(String s) {
        if (s == null) return null;
        String t = s.trim();
        if (t.isEmpty()) return null;
        int i = 0;
        int n = t.length();

        // Optional leading '+'
        char c0 = t.charAt(0);
        if (c0 == '+') {
            i++;
            if (i >= n) return null; // "+" only is invalid
        } else if (c0 == '-') {
            return null; // negative indices not allowed
        }

        int result = 0;
        while (i < n) {
            char ch = t.charAt(i++);
            if (ch < '0' || ch > '9') return null;
            int digit = ch - '0';
            if (result > (Integer.MAX_VALUE - digit) / 10) {
                return null; // overflow
            }
            result = result * 10 + digit;
        }
        return result;
    }

    // Prints the argument at the INDEX specified by indexStr within args.
    // Returns true on success, false on validation failure.
    public static boolean printArgAtIndex(String[] args, String indexStr) {
        if (args == null || args.length == 0 || indexStr == null) {
            System.err.println("ERROR");
            return false;
        }
        Integer idx = stringToInt(indexStr);
        if (idx == null) {
            System.err.println("ERROR");
            return false;
        }
        if (idx < 0 || idx >= args.length) {
            System.err.println("ERROR");
            return false;
        }
        // Print precisely the target argument
        System.out.println(args[idx]);
        return true;
    }

    private static void runTests() {
        // 5 test cases
        String[][] tests = new String[][]{
            new String[]{"x", "2", "a", "b", "c", "d"},       // prints "a"
            new String[]{"index", "0", "hello"},              // prints "index"
            new String[]{"index", "3", "p", "q", "r", "s"},   // prints "s"
            new String[]{"cmd", "100", "a", "b"},             // out of bounds -> ERROR (stderr)
            new String[]{"cmd", "abc", "z"}                   // invalid index -> ERROR (stderr)
        };
        for (String[] t : tests) {
            // the second argument (index 1) is the index string
            if (t.length > 1) {
                printArgAtIndex(t, t[1]);
            } else {
                System.err.println("ERROR");
            }
        }
    }

    public static void main(String[] args) {
        // Run 5 test cases
        runTests();

        // If invoked with real command line arguments, also process them:
        // Here, 'args[1]' (second argument) is the index string.
        if (args != null && args.length > 1) {
            printArgAtIndex(args, args[1]);
        }
    }
}