import java.util.Arrays;

public class Task71 {
    static class Result {
        final boolean success;
        final int value;
        final String error;

        Result(boolean success, int value, String error) {
            this.success = success;
            this.value = value;
            this.error = error;
        }
    }

    public static Result convertToInt(String input) {
        if (input == null) {
            return new Result(false, 0, "null input");
        }
        String s = input.trim();
        if (s.isEmpty()) {
            return new Result(false, 0, "empty input");
        }

        int sign = 1;
        int idx = 0;
        char first = s.charAt(0);
        if (first == '+' || first == '-') {
            sign = (first == '-') ? -1 : 1;
            idx++;
        }
        if (idx >= s.length()) {
            return new Result(false, 0, "no digits");
        }

        long acc = 0L;
        long limit = (sign == 1) ? Integer.MAX_VALUE : -(long) Integer.MIN_VALUE; // 2147483648 for negative
        for (; idx < s.length(); idx++) {
            char c = s.charAt(idx);
            if (c < '0' || c > '9') {
                return new Result(false, 0, "invalid character at position " + idx);
            }
            int digit = c - '0';
            if (acc > (limit - digit) / 10L) {
                return new Result(false, 0, "overflow");
            }
            acc = acc * 10L + digit;
        }
        int value = (sign == 1) ? (int) acc : (int) (-acc);
        return new Result(true, value, null);
    }

    public static void main(String[] args) {
        String[] inputs = (args != null && args.length > 0)
                ? args
                : new String[] { "123", "-42", "2147483647", "-2147483648", "99abc" };

        for (String s : inputs) {
            Result r = convertToInt(s);
            if (r.success) {
                System.out.println("Input: \"" + s + "\" -> OK " + r.value);
            } else {
                System.out.println("Input: \"" + s + "\" -> ERROR " + r.error);
            }
        }
    }
}