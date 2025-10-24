import java.math.BigDecimal;
import java.math.MathContext;
import java.util.Locale;

public class Task44 {

    public static String processInput(String input) {
        // Validation: null and length
        if (input == null) {
            return "ERROR: input is null";
        }
        if (input.length() > 1000) {
            return "ERROR: input too long";
        }

        // Validation: allowed characters only
        String allowedPattern = "^[\\s,\\d+\\-]*$";
        if (!input.matches(allowedPattern)) {
            return "ERROR: invalid characters present";
        }

        // Tokenize
        String trimmed = input.trim();
        if (trimmed.isEmpty()) {
            return "ERROR: empty input";
        }
        String[] rawTokens = trimmed.split("[,\\s]+");
        long count = 0L;
        long min = Long.MAX_VALUE;
        long max = Long.MIN_VALUE;
        long sum = 0L;

        for (String tok : rawTokens) {
            if (tok.isEmpty()) continue;
            if (++count > 100) {
                return "ERROR: too many numbers (limit 100)";
            }
            // Validate token numeric form
            if (!tok.matches("^[+-]?\\d{1,19}$")) {
                // Still try parse to catch boundary issues uniformly
                // but reject clearly invalid formats
                try {
                    Long.parseLong(tok);
                } catch (NumberFormatException nfe) {
                    return "ERROR: invalid number: " + tok;
                }
            }
            long val;
            try {
                val = Long.parseLong(tok);
            } catch (NumberFormatException ex) {
                return "ERROR: number out of range: " + tok;
            }

            // Update min/max
            if (val < min) min = val;
            if (val > max) max = val;

            // Safe sum with overflow detection
            try {
                sum = Math.addExact(sum, val);
            } catch (ArithmeticException ae) {
                return "ERROR: sum overflow";
            }
        }

        if (count == 0) {
            return "ERROR: no numbers found";
        }

        BigDecimal avg = new BigDecimal(sum).divide(BigDecimal.valueOf(count), MathContext.DECIMAL64);
        String avgStr = avg.stripTrailingZeros().toPlainString();

        return String.format(Locale.ROOT, "OK: count=%d, sum=%d, min=%d, max=%d, average=%s", count, sum, min, max, avgStr);
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "1, 2, 3, 4, 5",
            " 10 20 30 ",
            "-5, -10, 15",
            "abc",
            "9223372036854775807, 1"
        };
        for (String t : tests) {
            System.out.println("Input: \"" + t + "\" -> " + processInput(t));
        }
    }
}