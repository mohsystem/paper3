public class Lse19 {
    public static long add1000(long x) {
        return x + 1000;
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            try {
                long val = Long.parseLong(args[0]);
                long res = add1000(val);
                System.out.println(res);
            } catch (NumberFormatException e) {
                System.err.println("Invalid number: " + args[0]);
            }
        }
        long[] tests = new long[] {
            -1000L,
            0L,
            42L,
            999_999_999_999L,
            9_223_372_036_854_774_807L
        };
        for (long t : tests) {
            System.out.println(add1000(t));
        }
    }
}