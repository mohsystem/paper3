// Chain-of-Through process:
// 1) Problem understanding: Implement a robust parser that accepts a single input string with the format "OP A B" where OP ∈ {ADD,SUB,MUL,DIV} and A,B are 64-bit signed integers. Perform the operation securely and return "Result: <value>" or "Error: <reason>".
// 2) Security requirements: Validate input length, null checks, strict token count, strict operation whitelist, safe integer parsing, and overflow checks for all arithmetic, protected division, and no unsafe reflection or external calls.
// 3) Secure coding generation: Implement helper methods for safe arithmetic (add, sub, mul, div), input validation, and controlled parsing. Avoid unpredictable behavior and handle all error cases.
// 4) Code review: Checked for integer overflow, division by zero, input validation, error messages without sensitive data, no resource misuse.
// 5) Secure code output: Final code below reflects mitigations and robust error handling.

import java.util.Locale;

public class Task45 {

    // Public API: Accepts input string "OP A B" and returns a result or error message.
    public static String processInput(String input) {
        // Validation: null and length
        if (input == null) {
            return "Error: input is null";
        }
        String trimmed = input.trim();
        if (trimmed.length() == 0) {
            return "Error: input is empty";
        }
        if (trimmed.length() > 1000) {
            return "Error: input too long";
        }

        // Tokenize safely (expect exactly 3 tokens)
        String[] parts = trimmed.split("\\s+");
        if (parts.length != 3) {
            return "Error: expected format 'OP A B' with exactly three tokens";
        }

        String op = parts[0].toUpperCase(Locale.ROOT);
        long a, b;

        try {
            a = parseLongStrict(parts[1]);
        } catch (NumberFormatException nfe) {
            return "Error: invalid number for A";
        }

        try {
            b = parseLongStrict(parts[2]);
        } catch (NumberFormatException nfe) {
            return "Error: invalid number for B";
        }

        long[] result = new long[1];

        switch (op) {
            case "ADD":
                if (!safeAdd(a, b, result)) return "Error: overflow in addition";
                return "Result: " + result[0];
            case "SUB":
                if (!safeSub(a, b, result)) return "Error: overflow in subtraction";
                return "Result: " + result[0];
            case "MUL":
                if (!safeMul(a, b, result)) return "Error: overflow in multiplication";
                return "Result: " + result[0];
            case "DIV":
                if (!safeDiv(a, b, result)) return "Error: division error (division by zero or overflow)";
                return "Result: " + result[0];
            default:
                return "Error: unsupported operation";
        }
    }

    // Strict long parsing with bounds enforcement
    private static long parseLongStrict(String s) throws NumberFormatException {
        // Reject leading/trailing spaces (already trimmed by split)
        // Ensure only optional sign and digits
        if (!s.matches("[-+]?\\d+")) throw new NumberFormatException("Invalid digits");
        return Long.parseLong(s);
    }

    // Safe arithmetic helpers
    private static boolean safeAdd(long a, long b, long[] out) {
        if (b > 0 && a > Long.MAX_VALUE - b) return false;
        if (b < 0 && a < Long.MIN_VALUE - b) return false;
        out[0] = a + b;
        return true;
        }
    private static boolean safeSub(long a, long b, long[] out) {
        if (b > 0 && a < Long.MIN_VALUE + b) return false;
        if (b < 0 && a > Long.MAX_VALUE + b) return false;
        out[0] = a - b;
        return true;
    }
    private static boolean safeMul(long a, long b, long[] out) {
        if (a == 0 || b == 0) {
            out[0] = 0;
            return true;
        }
        if (a == -1 && b == Long.MIN_VALUE) return false;
        if (b == -1 && a == Long.MIN_VALUE) return false;

        // Use division-based overflow check
        if (a > 0) {
            if (b > 0) {
                if (a > Long.MAX_VALUE / b) return false;
            } else {
                if (b < Long.MIN_VALUE / a) return false;
            }
        } else { // a < 0
            if (b > 0) {
                if (a < Long.MIN_VALUE / b) return false;
            } else { // b < 0
                if (a != 0 && b < Long.MAX_VALUE / a) return false;
            }
        }
        out[0] = a * b;
        return true;
    }
    private static boolean safeDiv(long a, long b, long[] out) {
        if (b == 0) return false;
        if (a == Long.MIN_VALUE && b == -1) return false; // overflow
        out[0] = a / b;
        return true;
    }

    // Demonstration main with 5 test cases
    public static void main(String[] args) {
        String[] tests = new String[] {
            "ADD 5 7",
            "DIV 10 0",
            "MUL 9223372036854775807 2",
            "SUB -5 -10",
            "FOO 1 2"
        };

        for (String t : tests) {
            String res = processInput(t);
            System.out.println("Input: \"" + t + "\" -> " + res);
        }
    }
}