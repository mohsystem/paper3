import java.util.ArrayList;
import java.util.List;

public class Task71 {

    public static String tryParseInt(String input) {
        if (input == null) {
            return "ERROR: null input";
        }

        String s = input.trim();
        int len = s.length();
        if (len < 1 || len > 64) {
            return "ERROR: input length out of range (1..64)";
        }

        int idx = 0;
        int sign = 1;
        char first = s.charAt(0);
        if (first == '+' || first == '-') {
            sign = (first == '-') ? -1 : 1;
            idx++;
        }

        if (idx >= len) {
            return "ERROR: no digits found";
        }

        long value = 0L;
        for (; idx < len; idx++) {
            char c = s.charAt(idx);
            if (c < '0' || c > '9') {
                return "ERROR: non-digit character encountered";
            }
            int d = c - '0';
            if (sign == 1) {
                if (value > (Integer.MAX_VALUE - d) / 10L) {
                    return "ERROR: integer overflow";
                }
            } else {
                long limitNeg = 2147483648L; // -(long)Integer.MIN_VALUE
                if (value > (limitNeg - d) / 10L) {
                    return "ERROR: integer underflow";
                }
            }
            value = value * 10L + d;
        }

        int result = (sign == 1) ? (int) value : (int) -value;
        return "OK: " + result;
    }

    public static void main(String[] args) {
        List<String> inputs = new ArrayList<>();
        if (args != null && args.length > 0) {
            for (String a : args) {
                inputs.add(a);
            }
        } else {
            // 5 test cases
            inputs.add("123");
            inputs.add("-42");
            inputs.add("0042");
            inputs.add("2147483648");
            inputs.add("12a");
        }

        for (String in : inputs) {
            String res = tryParseInt(in);
            System.out.println("Input: \"" + in + "\" -> " + res);
        }
    }
}