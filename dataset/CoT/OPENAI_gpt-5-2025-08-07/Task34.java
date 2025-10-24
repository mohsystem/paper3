// Chain-of-Through process:
// 1) Problem understanding: Implement a function to check if a number is a Narcissistic (Armstrong) number in base 10.
// 2) Security requirements: Use safe integer operations, avoid floating-point pow to prevent precision issues.
// 3) Secure coding generation: Use integer exponentiation by squaring; guard against overflow by early checks.
// 4) Code review: Validate that additions cannot overflow by comparing against n before summing.
// 5) Secure code output: Finalized with tests; function accepts input as parameter and returns boolean.

public class Task34 {
    // Integer power using exponentiation by squaring
    private static long ipow(int base, int exp) {
        long result = 1L;
        long b = base;
        int e = exp;
        while (e > 0) {
            if ((e & 1) == 1) {
                result *= b;
            }
            b *= b;
            e >>= 1;
        }
        return result;
    }

    // Check if a number is Narcissistic in base 10
    public static boolean narcissistic(long n) {
        if (n <= 0) return false; // Only positive non-zero integers per problem
        String s = Long.toString(n);
        int k = s.length();
        long sum = 0L;

        for (int i = 0; i < k; i++) {
            int d = s.charAt(i) - '0';
            long term = ipow(d, k);

            // Early overflow-safe pruning:
            if (term > n) return false;
            if (sum > n - term) return false;

            sum += term;
        }
        return sum == n;
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        long[] tests = {153L, 1652L, 9474L, 7L, 10L};
        for (long t : tests) {
            System.out.println("n=" + t + " -> " + narcissistic(t));
        }
    }
}