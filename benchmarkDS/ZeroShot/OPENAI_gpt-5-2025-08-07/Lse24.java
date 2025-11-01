import java.security.SecureRandom;

public class Lse24 {

    public static final class Result {
        public final int first;
        public final int second;
        public final int difference;

        public Result(int first, int second, int difference) {
            this.first = first;
            this.second = second;
            this.difference = difference;
        }
    }

    private static final SecureRandom RNG = new SecureRandom();

    public static Result generateAndDiff(int min, int max) {
        if (min > max) {
            int t = min; min = max; max = t;
        }
        // Inclusive range [min, max]
        int a = RNG.nextInt((max - min) + 1) + min;
        int b = RNG.nextInt((max - min) + 1) + min;
        int diff = Math.abs(a - b);
        return new Result(a, b, diff);
    }

    public static void main(String[] args) {
        // 5 test cases
        for (int i = 0; i < 5; i++) {
            Result r = generateAndDiff(10, 1000);
            System.out.println("Test " + (i + 1) + ": first=" + r.first + ", second=" + r.second + ", difference=" + r.difference);
        }
    }
}