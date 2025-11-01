import java.security.SecureRandom;

public class Lse22 {
    private static final SecureRandom RNG = new SecureRandom();

    public static long generateRandomGreaterThan(long minExclusive) {
        if (minExclusive >= Long.MAX_VALUE) {
            throw new IllegalArgumentException("minExclusive too large");
        }
        long x;
        do {
            x = RNG.nextLong() & Long.MAX_VALUE;
        } while (x <= minExclusive);
        return x;
    }

    public static void main(String[] args) {
        for (int i = 0; i < 5; i++) {
            long v = generateRandomGreaterThan(1000L);
            System.out.println(v);
        }
    }
}