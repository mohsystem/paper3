// Chain-of-Through process in code generation:
// 1) Problem understanding: Generate a random number larger than 1000 and print it.
// 2) Security requirements: Use a strong random source and validate bounds.
// 3) Secure coding generation: Implement with SecureRandom, safe bounds, and overflow checks.
// 4) Code review: Ensure no overflow, handle edge cases, no insecure randomness.
// 5) Secure code output: Finalized after review.

import java.security.SecureRandom;

public class Lse22 {
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    // Generates a random integer strictly greater than minExclusive and at least > 1000.
    // Returns the generated integer.
    public static int generateRandomAbove(int minExclusive) {
        long min = Math.max(1000L, (long) minExclusive) + 1L;
        if (min < 0) min = 1001L; // defensive clamp
        long upper = Math.min((long) Integer.MAX_VALUE, min + 1_000_000L);
        if (upper < min) upper = min; // clamp to avoid negative bound
        int bound = (int) Math.max(1L, (upper - min + 1L));
        int offset = SECURE_RANDOM.nextInt(bound);
        return (int) (min + offset);
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        int[] tests = new int[]{-1, 0, 1000, 5000, 1_000_000};
        for (int t : tests) {
            int val = generateRandomAbove(t);
            System.out.println(val);
        }
    }
}