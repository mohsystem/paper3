import java.util.Arrays;

public class Task198 {
    // Chain-of-Through Step 1: Problem understanding - Implement rand10() using only rand7().
    // Chain-of-Through Step 2-4: Security - no external/random APIs; deterministic internal PRNG; rejection sampling to avoid modulo bias.
    // Chain-of-Through Step 5: Final secure implementation.

    // Simple xorshift64 PRNG (not using built-in randomness)
    private static long rngState = 0x123456789ABCDEFL; // non-zero seed

    private static long next64() {
        long x = rngState;
        x ^= (x << 13);
        x ^= (x >>> 7);
        x ^= (x << 17);
        rngState = x;
        return x & 0x7fffffffffffffffL; // 63-bit non-negative
    }

    // Uniform rand7() in [1..7] via rejection to avoid modulo bias
    public static int rand7() {
        final int limit = 2147483646; // 7 * floor(2^31 / 7) - ensures divisibility by 7
        while (true) {
            int v = (int) (next64() & 0x7fffffffL); // 31-bit non-negative
            if (v <= limit) {
                return (v % 7) + 1;
            }
        }
    }

    // rand10() using rand7() with rejection sampling
    public static int rand10() {
        while (true) {
            int a = rand7();
            int b = rand7();
            int idx = (a - 1) * 7 + b; // 1..49 uniformly
            if (idx <= 40) {
                return 1 + (idx - 1) % 10; // 1..10
            }
            // Optional layered reuse could be added, but simple rejection suffices
        }
    }

    // Run n trials and return results
    public static int[] runTest(int n) {
        if (n < 0) throw new IllegalArgumentException("n must be non-negative");
        int[] out = new int[n];
        for (int i = 0; i < n; i++) out[i] = rand10();
        return out;
    }

    private static void printArray(int[] arr) {
        StringBuilder sb = new StringBuilder();
        sb.append('[');
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) sb.append(',');
            sb.append(arr[i]);
        }
        sb.append(']');
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 3, 5, 20}; // 5 test cases
        for (int n : tests) {
            int[] res = runTest(n);
            printArray(res);
        }
    }
}