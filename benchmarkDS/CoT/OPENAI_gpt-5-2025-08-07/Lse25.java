// Chain-of-Through process:
// 1) Problem understanding: Generate 12 monthly sales (0..100000), sum by quarter, print results.
// 2) Security requirements: Use safe RNG (SplittableRandom) with explicit seeding, no shared mutable global state.
// 3) Secure coding generation: Encapsulate RNG, validate inputs, avoid overflow (values well within int range).
// 4) Code review: No unchecked external input, no I/O vulnerabilities, deterministic tests via seeds.
// 5) Secure code output: Final code ensures reproducibility and safe operations.

import java.util.SplittableRandom;

public class Lse25 {
    // Returns a random monthly sales value in [0, 100000], using provided RNG.
    public static int getMonthlySales(SplittableRandom rng) {
        // SplittableRandom nextInt origin-inclusive, bound-exclusive when using (origin, bound)
        return rng.nextInt(0, 100001);
    }

    // Computes quarterly sales sums for a single year using a provided seed.
    // Returns an array of length 4 where each entry is the sum for the corresponding quarter.
    public static int[] computeQuarterlySales(long seed) {
        SplittableRandom rng = new SplittableRandom(seed);
        int[] quarters = new int[4];
        for (int month = 0; month < 12; month++) {
            int sale = getMonthlySales(rng);
            quarters[month / 3] += sale;
        }
        return quarters;
    }

    // Prints quarterly results safely.
    public static void printQuarterResults(long seed, int[] quarters) {
        if (quarters == null || quarters.length != 4) {
            throw new IllegalArgumentException("quarters must be an array of length 4");
        }
        System.out.println("Seed " + seed + " quarterly totals:");
        for (int i = 0; i < 4; i++) {
            System.out.println("  Q" + (i + 1) + ": " + quarters[i]);
        }
    }

    // Main with 5 test cases.
    public static void main(String[] args) {
        long[] seeds = new long[] {0L, 1L, 42L, 2025L, 999999L};
        for (long seed : seeds) {
            int[] q = computeQuarterlySales(seed);
            printQuarterResults(seed, q);
        }
    }
}