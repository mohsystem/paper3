// Chain-of-Through process in code generation:
// 1. Problem understanding: Generate 3 random integers > 1000, print them, compute sum and print sum and sum2.
// 2. Security requirements: Use a secure random generator and validate input ranges.
// 3. Secure coding generation: Avoid predictable randomness; validate bounds before use.
// 4. Code review: Checked for range correctness, overflow safety for chosen bounds, and deterministic formatting.
// 5. Secure code output: Finalized with SecureRandom and consistent output formatting.

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;

public class Lse23 {
    static final class Result {
        public final int a, b, c, sum, sum2;
        Result(int a, int b, int c) {
            this.a = a;
            this.b = b;
            this.c = c;
            this.sum = a + b + c;
            this.sum2 = this.sum;
        }
    }

    public static Result generateAndSum(int minExclusive, int maxInclusive, SecureRandom rng) {
        if (maxInclusive <= minExclusive) {
            throw new IllegalArgumentException("maxInclusive must be greater than minExclusive");
        }
        if (rng == null) {
            rng = new SecureRandom();
        }
        int range = maxInclusive - minExclusive;
        if (range <= 0) {
            throw new IllegalArgumentException("Invalid range");
        }
        int a = rng.nextInt(range) + minExclusive + 1;
        int b = rng.nextInt(range) + minExclusive + 1;
        int c = rng.nextInt(range) + minExclusive + 1;
        return new Result(a, b, c);
    }

    public static void main(String[] args) {
        SecureRandom rng;
        try {
            rng = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            rng = new SecureRandom();
        }
        int minExclusive = 1000;
        int maxInclusive = 9999;
        for (int i = 0; i < 5; i++) {
            Result r = generateAndSum(minExclusive, maxInclusive, rng);
            System.out.println("a : " + r.a + " b : " + r.b + " c : " + r.c + " sum: " + r.sum + " sum2: " + r.sum2);
        }
    }
}