import java.security.SecureRandom;

public class Lse23 {
    private static final SecureRandom RNG = new SecureRandom();
    private static final int DEFAULT_MIN = 1001;
    private static final int DEFAULT_MAX = 1_000_000; // inclusive

    // Generates three random integers >= minInclusive (and <= DEFAULT_MAX) and returns the formatted output line.
    public static String generateOutput(int minInclusive) {
        int lower = Math.max(minInclusive, DEFAULT_MIN);
        int upper = DEFAULT_MAX;
        if (lower > upper) {
            lower = DEFAULT_MIN; // fail closed to a safe default range
        }

        int a = lower + RNG.nextInt(upper - lower + 1);
        int b = lower + RNG.nextInt(upper - lower + 1);
        int c = lower + RNG.nextInt(upper - lower + 1);

        long sum = (long) a + (long) b + (long) c;
        long sum2 = (long) a + (long) b + (long) c;

        return "a : " + a + " b : " + b + " c : " + c + " sum: " + sum + " sum2: " + sum2;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(generateOutput(1001));
        System.out.println(generateOutput(1001));
        System.out.println(generateOutput(1001));
        System.out.println(generateOutput(1001));
        System.out.println(generateOutput(1001));
    }
}