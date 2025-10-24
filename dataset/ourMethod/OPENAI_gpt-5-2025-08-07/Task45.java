import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task45 {

    private static final long MIN_VAL = -1_000_000_000L;
    private static final long MAX_VAL = 1_000_000_000L;
    private static final int MAX_INPUT_LEN = 100;

    public static String processCommand(String input) {
        try {
            if (input == null) {
                return "ERROR: input is null";
            }
            // Validate input length and encoding
            byte[] bytes = input.getBytes(StandardCharsets.UTF_8);
            if (bytes.length == 0 || bytes.length > MAX_INPUT_LEN) {
                return "ERROR: input length out of allowed range (1..100 bytes)";
            }

            String trimmed = input.trim();
            String[] parts = trimmed.split("\\s+");
            if (parts.length != 3) {
                return "ERROR: expected format '<OP> <A> <B>' with exactly 3 tokens";
            }

            String op = parts[0];
            String aStr = parts[1];
            String bStr = parts[2];

            if (!isUpperAlpha(op)) {
                return "ERROR: operation must be uppercase letters only";
            }

            if (!isValidOp(op)) {
                return "ERROR: unsupported operation";
            }

            Long a = parseLongSafe(aStr);
            Long b = parseLongSafe(bStr);
            if (a == null || b == null) {
                return "ERROR: operands must be valid 64-bit integers";
            }

            if (a < MIN_VAL || a > MAX_VAL || b < MIN_VAL || b > MAX_VAL) {
                return "ERROR: operands out of allowed range [-1000000000, 1000000000]";
            }

            Long result;

            switch (op) {
                case "ADD":
                    result = safeAdd(a, b);
                    if (result == null) return "ERROR: addition overflow";
                    break;
                case "SUB":
                    result = safeSub(a, b);
                    if (result == null) return "ERROR: subtraction overflow";
                    break;
                case "MUL":
                    result = safeMul(a, b);
                    if (result == null) return "ERROR: multiplication overflow";
                    break;
                case "DIV":
                    if (b == 0) return "ERROR: division by zero";
                    if (a == Long.MIN_VALUE && b == -1) return "ERROR: division overflow";
                    result = a / b;
                    break;
                case "MOD":
                    if (b == 0) return "ERROR: modulo by zero";
                    result = a % b;
                    break;
                case "POW":
                    if (b < 0) return "ERROR: negative exponent not supported";
                    // Limit exponent to mitigate overflow; still check during computation
                    if (b > 62) return "ERROR: exponent too large";
                    result = safePow(a, b);
                    if (result == null) return "ERROR: power overflow";
                    break;
                default:
                    return "ERROR: unsupported operation";
            }

            return "OK result: " + result;
        } catch (Exception e) {
            return "ERROR: unexpected failure";
        }
    }

    private static boolean isUpperAlpha(String s) {
        if (s == null || s.isEmpty() || s.length() > 10) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c < 'A' || c > 'Z') return false;
        }
        return true;
    }

    private static boolean isValidOp(String op) {
        return Arrays.asList("ADD", "SUB", "MUL", "DIV", "MOD", "POW").contains(op);
    }

    private static Long parseLongSafe(String s) {
        if (s == null || s.isEmpty() || s.length() > 20) return null;
        try {
            return Long.parseLong(s);
        } catch (NumberFormatException e) {
            return null;
        }
    }

    private static Long safeAdd(long a, long b) {
        long r = a + b;
        if (((a ^ r) & (b ^ r)) < 0) return null; // overflow detection
        return r;
    }

    private static Long safeSub(long a, long b) {
        long r = a - b;
        if (((a ^ b) & (a ^ r)) < 0) return null;
        return r;
    }

    private static Long safeMul(long a, long b) {
        if (a == 0 || b == 0) return 0L;
        // Check overflow before multiplication
        if (a == Long.MIN_VALUE && b == -1) return null;
        long r = a * b;
        if (a != 0 && r / a != b) return null;
        return r;
    }

    private static Long safePow(long base, long exp) {
        long result = 1;
        long b = base;
        long e = exp;
        while (e > 0) {
            if ((e & 1L) == 1L) {
                Long tmp = safeMul(result, b);
                if (tmp == null) return null;
                result = tmp;
            }
            e >>= 1;
            if (e > 0) {
                Long tmp2 = safeMul(b, b);
                if (tmp2 == null) return null;
                b = tmp2;
            }
        }
        return result;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "ADD 3 5",
            "DIV 10 0",
            "MUL 123456 789",
            "POW 2 10",
            "XYZ 1 2"
        };
        for (String t : tests) {
            try {
                String out = processCommand(t);
                System.out.println("Input: '" + t + "' -> " + out);
            } catch (Exception ex) {
                System.out.println("Input: '" + t + "' -> ERROR: unexpected failure");
            }
        }
    }
}