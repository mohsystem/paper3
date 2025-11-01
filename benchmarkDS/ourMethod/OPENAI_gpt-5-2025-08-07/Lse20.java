import java.nio.charset.StandardCharsets;

public class Lse20 {
    private static final long OFFSET = 1000L;

    public static long addOffset(long value) {
        if (value > Long.MAX_VALUE - OFFSET) {
            throw new ArithmeticException("overflow");
        }
        return value + OFFSET;
    }

    private static boolean tryParseLong(String s, long[] out) {
        if (s == null) return false;
        String t = s.trim();
        if (t.isEmpty() || t.length() > 20) return false; // optional sign + up to 19 digits
        if (!t.matches("^[+-]?\\d{1,19}$")) return false;
        try {
            long v = Long.parseLong(t);
            out[0] = v;
            return true;
        } catch (NumberFormatException ex) {
            return false;
        }
    }

    public static void main(String[] args) {
        if (args.length >= 1) {
            long[] parsed = new long[1];
            if (!tryParseLong(args[0], parsed)) {
                System.err.println("Invalid input");
                System.exit(1);
            }
            long input = parsed[0];
            try {
                long result = addOffset(input);
                System.out.println(result);
            } catch (ArithmeticException ex) {
                System.err.println("Overflow");
                System.exit(1);
            }
        } else {
            // Run 5 test cases
            long[] tests = new long[] {0L, 42L, -1000L, Long.MAX_VALUE - OFFSET, Long.MIN_VALUE + OFFSET};
            for (long v : tests) {
                try {
                    long r = addOffset(v);
                    System.out.println(r);
                } catch (ArithmeticException ex) {
                    System.out.println("Overflow");
                }
            }
        }
    }
}