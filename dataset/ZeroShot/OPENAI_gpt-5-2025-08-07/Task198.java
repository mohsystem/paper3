import java.util.ArrayList;
import java.util.List;

public class Task198 {
    // Simple xorshift32 PRNG for reproducible, deterministic "rand7" emulation without built-in randomness
    static final class RNG {
        private int state;

        public RNG(long seed) {
            // Mix the seed to avoid zero and poor seeds
            long z = seed + 0x9E3779B97F4A7C15L;
            z = (z ^ (z >>> 30)) * 0xBF58476D1CE4E5B9L;
            z = (z ^ (z >>> 27)) * 0x94D049BB133111EBL;
            z ^= (z >>> 31);
            this.state = (int) z;
            if (this.state == 0) {
                this.state = 0x6D2B79F5; // Non-zero default
            }
        }

        private int nextUInt32() {
            int x = state;
            x ^= (x << 13);
            x ^= (x >>> 17);
            x ^= (x << 5);
            state = x;
            return x;
        }

        // Returns a uniform integer in [1,7]
        public int rand7() {
            final long LIMIT = 4294967291L; // floor((2^32)/7)*7 - 1 = 4294967291
            while (true) {
                long u = Integer.toUnsignedLong(nextUInt32());
                if (u <= LIMIT) {
                    return 1 + (int) (u % 7L);
                }
            }
        }

        // Returns a uniform integer in [1,10] using only rand7()
        public int rand10() {
            while (true) {
                int a = rand7();
                int b = rand7();
                int idx = (a - 1) * 7 + b; // uniform in [1,49]
                if (idx <= 40) {
                    return 1 + (idx - 1) % 10;
                }
                // Optional optimization reusing the overflow can be added, but simple rejection is fine
            }
        }
    }

    // Runs n times rand10() and returns the results
    public static List<Integer> runTest(int n, long seed) {
        if (n < 0) throw new IllegalArgumentException("n must be non-negative");
        RNG rng = new RNG(seed);
        List<Integer> res = new ArrayList<>(n);
        for (int i = 0; i < n; i++) {
            res.add(rng.rand10());
        }
        return res;
    }

    private static void printList(List<Integer> list) {
        StringBuilder sb = new StringBuilder();
        sb.append('[');
        for (int i = 0; i < list.size(); i++) {
            if (i > 0) sb.append(',');
            sb.append(list.get(i));
        }
        sb.append(']');
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 3, 5, 10};
        long[] seeds = {12345L, 67890L, 13579L, 24680L, 424242L};
        for (int i = 0; i < tests.length; i++) {
            List<Integer> out = runTest(tests[i], seeds[i]);
            printList(out);
        }
    }
}