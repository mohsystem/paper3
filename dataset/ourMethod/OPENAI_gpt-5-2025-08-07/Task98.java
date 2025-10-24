import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

public class Task98 {
    private static final int MAX_LEN = 64;

    public static final class ParseResult {
        public final boolean success;
        public final int value;
        public final String error;

        private ParseResult(boolean success, int value, String error) {
            this.success = success;
            this.value = value;
            this.error = error;
        }

        public static ParseResult ok(int value) {
            return new ParseResult(true, value, "");
        }

        public static ParseResult err(String error) {
            return new ParseResult(false, 0, error);
        }
    }

    public static ParseResult parseIntStrict(String input) {
        if (input == null) {
            return ParseResult.err("Input is null");
        }

        String trimmed = input.trim();
        int len = trimmed.length();
        if (len == 0) {
            return ParseResult.err("Empty input");
        }
        if (len > MAX_LEN) {
            return ParseResult.err("Input too long");
        }

        int idx = 0;
        boolean negative = false;
        char first = trimmed.charAt(0);
        if (first == '+' || first == '-') {
            negative = (first == '-');
            idx++;
            if (idx >= len) {
                return ParseResult.err("No digits after sign");
            }
        }

        long acc = 0L;
        for (int i = idx; i < len; i++) {
            char c = trimmed.charAt(i);
            if (c < '0' || c > '9') {
                return ParseResult.err("Invalid character encountered");
            }
            int d = c - '0';
            acc = acc * 10L + d;
            long limit = negative ? (-(long)Integer.MIN_VALUE) : (long)Integer.MAX_VALUE;
            if (acc > limit) {
                return ParseResult.err("Integer out of range");
            }
        }

        long signedVal = negative ? -acc : acc;
        if (signedVal < Integer.MIN_VALUE || signedVal > Integer.MAX_VALUE) {
            return ParseResult.err("Integer out of range");
        }
        return ParseResult.ok((int) signedVal);
    }

    public static void main(String[] args) {
        try (BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8))) {
            for (int i = 1; i <= 5; i++) {
                System.out.print("Enter value #" + i + ": ");
                String line = br.readLine();
                if (line == null) {
                    System.out.println("No more input.");
                    break;
                }
                ParseResult res = parseIntStrict(line);
                if (res.success) {
                    System.out.println("Parsed integer: " + res.value);
                } else {
                    System.out.println("Error: " + res.error);
                }
            }
        } catch (IOException e) {
            System.out.println("I/O error: " + e.getMessage());
        }
    }
}