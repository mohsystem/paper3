public class Lse21 {
    public static long add1000(long value) {
        // Safe addition using addExact to detect overflow
        return Math.addExact(value, 1000L);
    }

    private static boolean isValidInteger(String s) {
        if (s == null) return false;
        String v = s.trim();
        return v.matches("^[+-]?\\d{1,19}$");
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            String input = args[0];
            if (!isValidInteger(input)) {
                System.err.println("Invalid input");
                System.exit(1);
            }
            try {
                long n = Long.parseLong(input.trim());
                long result = add1000(n);
                System.out.println(result);
                System.exit(0);
            } catch (NumberFormatException | ArithmeticException ex) {
                System.err.println("Invalid input");
                System.exit(1);
            }
        } else {
            // 5 test cases
            long[] tests = new long[] {
                0L,
                1L,
                -1000L,
                123456789012345L,
                Long.MAX_VALUE - 1000L
            };
            for (long t : tests) {
                try {
                    long r = add1000(t);
                    System.out.println(r);
                } catch (ArithmeticException ex) {
                    System.out.println("overflow");
                }
            }
            System.exit(0);
        }
    }
}