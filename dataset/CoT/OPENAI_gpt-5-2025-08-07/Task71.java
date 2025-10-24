import java.util.Arrays;

public class Task71 {

    // Secure integer parsing: returns null if invalid or out of range
    public static Integer parseToInt(String input) {
        if (input == null) return null;

        // Trim leading/trailing whitespace
        String s = input.trim();
        if (s.isEmpty()) return null;

        // Length guard to avoid excessive processing
        if (s.length() > 64) return null;

        int len = s.length();
        int i = 0;
        boolean negative = false;

        char first = s.charAt(0);
        if (first == '-') {
            negative = true;
            i = 1;
        } else if (first == '+') {
            i = 1;
        }

        if (i == len) return null; // Only sign provided

        int limit = negative ? Integer.MIN_VALUE : -Integer.MAX_VALUE;
        int multmin = limit / 10;
        int result = 0;

        for (; i < len; i++) {
            char ch = s.charAt(i);
            if (ch < '0' || ch > '9') return null;

            int digit = ch - '0';
            if (result < multmin) return null;
            result *= 10;
            if (result < limit + digit) return null;
            result -= digit;
        }

        return negative ? result : -result;
    }

    public static void main(String[] args) {
        String[] inputs = (args != null && args.length > 0)
                ? args
                : new String[]{"123", "-42", "00123", "2147483648", "abc123"};

        for (String s : inputs) {
            Integer val = parseToInt(s);
            if (val != null) {
                System.out.println("Input: \"" + s + "\" -> OK: " + val);
            } else {
                System.out.println("Input: \"" + s + "\" -> Invalid integer");
            }
        }
    }
}