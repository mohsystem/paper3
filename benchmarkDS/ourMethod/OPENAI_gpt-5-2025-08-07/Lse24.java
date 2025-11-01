import java.security.SecureRandom;

public class Lse24 {
    private static final SecureRandom RNG = new SecureRandom();

    public static final class Result {
        public final int first;
        public final int second;
        public final int difference;

        public Result(int first, int second, int difference) {
            this.first = first;
            this.second = second;
            this.difference = difference;
        }

        @Override
        public String toString() {
            return "First: " + first + ", Second: " + second + ", Difference (first - second): " + difference;
        }
    }

    // Generates two random integers in [10, 1000], computes difference, and returns results.
    public static Result generateAndDiff() {
        int first = RNG.nextInt(991) + 10;  // 10..1000 inclusive
        int second = RNG.nextInt(991) + 10; // 10..1000 inclusive
        int difference = first - second;
        return new Result(first, second, difference);
    }

    public static void main(String[] args) {
        for (int i = 1; i <= 5; i++) {
            Result r = generateAndDiff();
            System.out.println("Test " + i + ": " + r.toString());
        }
    }
}