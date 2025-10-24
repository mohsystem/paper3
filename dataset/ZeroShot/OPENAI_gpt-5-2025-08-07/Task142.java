import java.math.BigInteger;

public class Task142 {
    // Returns the greatest common divisor (GCD) of two 64-bit integers.
    // For the special unrepresentable magnitude case (|Long.MIN_VALUE|), returns Long.MIN_VALUE as a sentinel.
    public static long gcd(long a, long b) {
        if (a == 0L && b == 0L) {
            return 0L;
        }
        while (b != 0L) {
            long r = Math.floorMod(a, b);
            a = b;
            b = r;
        }
        if (a == Long.MIN_VALUE) {
            return Long.MIN_VALUE; // Sentinel when true absolute value is not representable.
        }
        return Math.abs(a);
    }

    // BigInteger GCD for arbitrary precision use cases.
    public static BigInteger gcdBig(BigInteger a, BigInteger b) {
        if (a == null || b == null) {
            throw new IllegalArgumentException("Inputs must not be null");
        }
        return a.gcd(b);
    }

    public static void main(String[] args) {
        long[][] tests = new long[][]{
            {48L, 18L},
            {0L, 0L},
            {-48L, 18L},
            {270L, 192L},
            {1234567890L, 9876543210L}
        };
        for (long[] t : tests) {
            long res = gcd(t[0], t[1]);
            System.out.println("gcd(" + t[0] + ", " + t[1] + ") = " + res);
        }
    }
}