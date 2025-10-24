import java.util.ArrayList;
import java.util.List;

public class Task198 {
    // Park-Miller LCG parameters
    private static final long MOD = 2147483647L; // 2^31 - 1
    private static final long MUL = 48271L;
    private static long seed = 123456789L;

    private static int rngNext() {
        seed = (seed * MUL) % MOD; // 1..MOD-1
        return (int) seed;
    }

    private static int rngUniform(int k) {
        if (k <= 0) throw new IllegalArgumentException("k must be positive");
        long m1 = MOD - 1L; // 2147483646
        long limit = (m1 / k) * k; // largest multiple of k less than or equal to m1
        while (true) {
            long r = (long) rngNext() - 1L; // 0..m1-1
            if (r < limit) {
                return (int) (r % k);
            }
        }
    }

    // Given API
    public static int rand7() {
        return rngUniform(7) + 1; // 1..7
    }

    // Required function
    public static int rand10() {
        while (true) {
            int a = rand7();
            int b = rand7();
            int num = (a - 1) * 7 + b; // 1..49
            if (num <= 40) {
                return 1 + (num - 1) % 10;
            }
        }
    }

    public static int[] runRand10NTimes(int n) {
        if (n < 1 || n > 100000) throw new IllegalArgumentException("n out of range");
        int[] res = new int[n];
        for (int i = 0; i < n; i++) res[i] = rand10();
        return res;
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
        int[] tests = {1, 2, 3, 5, 10};
        for (int n : tests) {
            int[] out = runRand10NTimes(n);
            printArray(out);
        }
    }
}