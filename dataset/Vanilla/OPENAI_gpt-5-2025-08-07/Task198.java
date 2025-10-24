import java.util.*;

public class Task198 {
    // Simple LCG (no built-in RNG)
    private static long seed = 123456789L;

    public static void resetSeed(long s) {
        seed = s;
    }

    private static int nextRand() {
        // 32-bit LCG: X_{n+1} = (a * X_n + c) mod 2^31
        seed = (seed * 1103515245L + 12345L) & 0x7fffffffL;
        return (int) seed;
    }

    // Given API
    public static int rand7() {
        return (nextRand() % 7) + 1;
    }

    // Use only rand7()
    public static int rand10() {
        while (true) {
            int a = rand7();
            int b = rand7();
            int num = (a - 1) * 7 + b; // 1..49
            if (num <= 40) {
                return ((num - 1) % 10) + 1;
            }
        }
    }

    // Generate n samples of rand10
    public static int[] generate(int n) {
        int[] out = new int[n];
        for (int i = 0; i < n; i++) out[i] = rand10();
        return out;
    }

    private static void printArray(int[] arr) {
        StringBuilder sb = new StringBuilder();
        sb.append("[");
        for (int i = 0; i < arr.length; i++) {
            if (i > 0) sb.append(",");
            sb.append(arr[i]);
        }
        sb.append("]");
        System.out.println(sb.toString());
    }

    public static void main(String[] args) {
        int[] tests = {1, 2, 3, 10, 20};
        for (int n : tests) {
            resetSeed(123456789L); // reset for reproducibility per test
            int[] res = generate(n);
            printArray(res);
        }
    }
}