import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Task98 {

    private static final Pattern INT_PATTERN = Pattern.compile("^[+-]?\\d+$");
    private static final BigInteger LONG_MIN = BigInteger.valueOf(Long.MIN_VALUE);
    private static final BigInteger LONG_MAX = BigInteger.valueOf(Long.MAX_VALUE);

    public static final class ParseResult {
        public final boolean ok;
        public final long value;

        public ParseResult(boolean ok, long value) {
            this.ok = ok;
            this.value = value;
        }
    }

    public static ParseResult tryParseLong(String input) {
        if (input == null) {
            return new ParseResult(false, 0L);
        }
        String t = input.strip();
        if (t.isEmpty() || !INT_PATTERN.matcher(t).matches()) {
            return new ParseResult(false, 0L);
        }
        try {
            BigInteger bi = new BigInteger(t);
            if (bi.compareTo(LONG_MIN) < 0 || bi.compareTo(LONG_MAX) > 0) {
                return new ParseResult(false, 0L);
            }
            return new ParseResult(true, bi.longValue());
        } catch (NumberFormatException ex) {
            return new ParseResult(false, 0L);
        }
    }

    public static void main(String[] args) throws IOException {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in, StandardCharsets.UTF_8));
        String line = br.readLine();
        if (line != null) {
            ParseResult result = tryParseLong(line);
            if (result.ok) {
                System.out.println("Parsed integer: " + result.value);
            } else {
                System.out.println("Invalid integer input.");
            }
        } else {
            System.out.println("No input provided.");
        }

        String[] tests = new String[]{
            "123",
            "   -456   ",
            "+789",
            "9223372036854775808",
            "abc123"
        };

        for (String test : tests) {
            ParseResult r = tryParseLong(test);
            if (r.ok) {
                System.out.println("Test \"" + test + "\" -> " + r.value);
            } else {
                System.out.println("Test \"" + test + "\" -> invalid");
            }
        }
    }
}