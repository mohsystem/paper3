import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

public class Task45 {

    public static String processInput(String input) {
        if (input == null) {
            return "ERROR: Empty input";
        }
        String trimmed = input.trim();
        if (trimmed.isEmpty()) {
            return "ERROR: Empty input";
        }
        String[] tokens = trimmed.split("\\s+");
        if (tokens.length != 3) {
            return "ERROR: Expected format: OP A B";
        }
        String op = tokens[0].toUpperCase(Locale.ROOT);
        long a, b;
        try {
            a = parseLongStrict(tokens[1]);
            b = parseLongStrict(tokens[2]);
        } catch (NumberFormatException ex) {
            return "ERROR: Invalid number";
        }

        try {
            switch (op) {
                case "ADD":
                    return "RESULT: " + Math.addExact(a, b);
                case "SUB":
                    return "RESULT: " + Math.subtractExact(a, b);
                case "MUL":
                    return "RESULT: " + Math.multiplyExact(a, b);
                case "DIV":
                    if (b == 0) return "ERROR: Division by zero";
                    if (a == Long.MIN_VALUE && b == -1) return "ERROR: Overflow";
                    return "RESULT: " + (a / b);
                case "MOD":
                    if (b == 0) return "ERROR: Modulus by zero";
                    return "RESULT: " + (a % b);
                case "POW":
                    if (b < 0) return "ERROR: Negative exponent";
                    if (a == 0 && b == 0) return "ERROR: Undefined 0^0";
                    try {
                        return "RESULT: " + powChecked(a, b);
                    } catch (ArithmeticException ex) {
                        return "ERROR: Overflow";
                    }
                default:
                    return "ERROR: Unknown operation";
            }
        } catch (ArithmeticException ex) {
            return "ERROR: Overflow";
        } catch (Exception ex) {
            return "ERROR: Unexpected error";
        }
    }

    private static long parseLongStrict(String s) {
        // Disallow leading/trailing spaces and ensure full parse
        String t = s.trim();
        if (!t.matches("[+-]?\\d+")) {
            throw new NumberFormatException("Invalid");
        }
        return Long.parseLong(t);
    }

    private static long powChecked(long base, long exp) {
        long result = 1L;
        long b = base;
        long e = exp;
        while (e > 0) {
            if ((e & 1L) == 1L) {
                result = Math.multiplyExact(result, b);
            }
            e >>= 1;
            if (e > 0) {
                b = Math.multiplyExact(b, b);
            }
        }
        return result;
    }

    public static void main(String[] args) {
        List<String> tests = new ArrayList<>();
        tests.add("ADD 3 5");
        tests.add("DIV 10 0");
        tests.add("POW 2 10");
        tests.add("MUL 9223372036854775807 2");
        tests.add("SUB a 5");

        for (String t : tests) {
            String out = processInput(t);
            System.out.println("Input: " + t + " -> " + out);
        }
    }
}