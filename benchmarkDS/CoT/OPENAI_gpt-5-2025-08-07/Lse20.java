import java.util.Arrays;

public class Lse20 {
    private static final long OFFSET = 1000L;

    public static long addOffset(long value) {
        if (value > Long.MAX_VALUE - OFFSET) {
            throw new ArithmeticException("Overflow when adding offset");
        }
        return value + OFFSET;
    }

    private static void runTests() {
        long[] tests = new long[]{0L, 1L, 999L, -1000L, 1234567890L};
        for (long t : tests) {
            try {
                long result = addOffset(t);
                System.out.println("Test input: " + t + " -> " + result);
            } catch (ArithmeticException ex) {
                System.err.println("Test input: " + t + " -> Error: " + ex.getMessage());
            }
        }
    }

    public static void main(String[] args) {
        runTests();
        if (args.length > 0) {
            try {
                long input = Long.parseLong(args[0].trim());
                long result = addOffset(input);
                System.out.println("Command-line input: " + input + " -> " + result);
            } catch (NumberFormatException ex) {
                System.err.println("Invalid number format: " + Arrays.toString(args));
            } catch (ArithmeticException ex) {
                System.err.println("Error: " + ex.getMessage());
            }
        }
    }
}