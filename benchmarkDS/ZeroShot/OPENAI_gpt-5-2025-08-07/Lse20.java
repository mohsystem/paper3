public class Lse20 {
    private static final long OFFSET = 1000L;

    public static long addOffset(long value) throws ArithmeticException {
        if (value > Long.MAX_VALUE - OFFSET) {
            throw new ArithmeticException("Overflow when adding offset");
        }
        if (value < Long.MIN_VALUE + OFFSET) {
            throw new ArithmeticException("Underflow when adding offset");
        }
        return value + OFFSET;
    }

    public static void main(String[] args) {
        if (args != null && args.length == 1) {
            try {
                String s = args[0] == null ? "" : args[0].trim();
                if (s.isEmpty()) {
                    System.err.println("Error: empty input");
                    System.exit(1);
                }
                long v = Long.parseLong(s);
                long res = addOffset(v);
                System.out.println(res);
            } catch (NumberFormatException e) {
                System.err.println("Error: invalid integer input");
                System.exit(1);
            } catch (ArithmeticException e) {
                System.err.println("Error: " + e.getMessage());
                System.exit(1);
            }
        } else {
            long[] tests = new long[] {
                0L,
                1L,
                -1L,
                Long.MAX_VALUE - 1000L,
                Long.MIN_VALUE + 1000L
            };
            for (long t : tests) {
                try {
                    long res = addOffset(t);
                    System.out.println("addOffset(" + t + ") = " + res);
                } catch (ArithmeticException e) {
                    System.out.println("addOffset(" + t + ") error: " + e.getMessage());
                }
            }
        }
    }
}