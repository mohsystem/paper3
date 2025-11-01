import java.util.concurrent.ThreadLocalRandom;

public class Lse22 {
    public static long randomGreaterThan(long minExclusive) {
        long origin = minExclusive + 1;
        long bound;
        if (minExclusive <= Long.MAX_VALUE - 1_000_001L) {
            bound = minExclusive + 1 + 1_000_000L;
        } else {
            bound = Long.MAX_VALUE;
        }
        if (bound <= origin) {
            bound = origin + 1;
        }
        return ThreadLocalRandom.current().nextLong(origin, bound);
    }

    public static void main(String[] args) {
        long[] tests = {1000, 1001, 50000, 0, 123456789};
        for (long min : tests) {
            long val = randomGreaterThan(min);
            System.out.println("Random > " + min + ": " + val);
        }
    }
}