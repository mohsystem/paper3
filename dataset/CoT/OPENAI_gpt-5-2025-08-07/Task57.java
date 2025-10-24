// Chain-of-Through process:
// 1) Problem understanding: Compute factorial of a non-negative integer n and return as String.
// 2) Security requirements: Validate input (non-negative), avoid recursion (stack overflow), handle large values using BigInteger.
// 3) Secure coding generation: Iterative multiplication, bounds check, no unsafe operations.
// 4) Code review: Ensure no overflows, proper handling of edge cases (n=0, n=1).
// 5) Secure code output: Finalized robust implementation with tests.

import java.math.BigInteger;

public class Task57 {
    // Computes factorial of n; returns "INVALID" for negative inputs.
    public static String computeFactorial(int n) {
        if (n < 0) {
            return "INVALID";
        }
        BigInteger result = BigInteger.ONE;
        for (int i = 2; i <= n; i++) {
            result = result.multiply(BigInteger.valueOf(i));
        }
        return result.toString();
    }

    public static void main(String[] args) {
        int[] tests = {0, 1, 5, 10, 20};
        for (int t : tests) {
            String ans = computeFactorial(t);
            System.out.println("factorial(" + t + ") = " + ans);
        }
    }
}