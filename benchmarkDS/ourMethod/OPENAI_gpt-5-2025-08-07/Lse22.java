import java.security.SecureRandom;

public class Lse22 {
    private static final SecureRandom RNG = new SecureRandom();
    private static final long MAX_SPAN = 1_000_000_000L; // cap the span to avoid overflow and keep ranges manageable

    // Generates a random long strictly greater than 'min'. Uses a capped upper bound for safe range handling.
    public static long generateRandomAbove(long min) {
        if (min == Long.MAX_VALUE) {
            throw new IllegalArgumentException("min too large");
        }
        final long lower = min + 1; // safe because we already ensured min != Long.MAX_VALUE
        final long remaining = Long.MAX_VALUE - lower; // >= 0
        final long span = Math.min(MAX_SPAN, remaining); // span in [0, MAX_SPAN]
        final long upper = lower + span; // upper >= lower
        final long count = span + 1; // number of values in [lower, upper], in [1, MAX_SPAN+1]

        return lower + boundedRandom(count);
    }

    // Returns a value in [0, bound) with rejection sampling to avoid modulo bias. Requires 1 <= bound <= MAX_SPAN+1
    private static long boundedRandom(long bound) {
        if (bound <= 0 || bound > (MAX_SPAN + 1)) {
            throw new IllegalArgumentException("invalid bound");
        }
        long r, m;
        do {
            r = RNG.nextLong() >>> 1; // 63-bit non-negative
            m = r % bound;
        } while (r - m + (bound - 1) < 0L); // rejection to remove modulo bias
        return m;
    }

    public static void main(String[] args) {
        long[] tests = new long[] {
            1000L,
            5000L,
            12345L,
            999_999L,
            Long.MAX_VALUE - 2L
        };

        for (long t : tests) {
            long v = generateRandomAbove(t);
            System.out.println("min=" + t + " -> random=" + v);
            if (v <= t) {
                throw new IllegalStateException("Generated value not greater than min");
            }
        }
    }
}