// Chain-of-Through Step 1: Problem understanding
// - Generate two random integers in [10, 1000], compute their difference, print results.
// - Provide a function that accepts parameters (min, max) and returns the outputs.
// Chain-of-Through Step 2: Security requirements
// - Use a cryptographically secure RNG (SecureRandom).
// - Validate bounds to avoid errors.
// Chain-of-Through Step 3: Secure coding generation
// - Implement parameter validation, avoid overflow, and compute absolute difference safely.
// Chain-of-Through Step 4: Code review
// - Ensure inclusive bounds, handle small ranges safely, and avoid predictable seeds.
// Chain-of-Through Step 5: Secure code output
// - Final implementation below.

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
            throw new IllegalArgumentException("min must be <= max");
        }
        int a = nextInRange(min, max);
        int b = nextInRange(min, max);
        int diff = Math.abs(a - b);
        return new Result(a, b, diff);
    }

    private static int nextInRange(int min, int max) {
        int span = max - min + 1; // safe since max and min are ints; here <= 991
        int val = RNG.nextInt(span) + min;
        return val;
    }

    public static void main(String[] args) {
        // 5 test cases as required
        for (int i = 1; i <= 5; i++) {
            Result r = generateAndDiff(10, 1000);
            System.out.println("Test " + i + ": First=" + r.first + ", Second=" + r.second + ", Difference=" + r.difference);
        }
    }
}