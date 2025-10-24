import java.math.BigInteger;

public class Task142 {
    // Chain-of-Through process:
    // 1) Problem: Compute GCD of two numbers.
    // 2) Security: Avoid unsafe inputs; handle negatives and zeros safely.
    // 3) Secure generation: Use BigInteger to avoid overflow and ensure correctness.
    // 4) Review: Iterative logic or BigInteger.gcd is safe and returns non-negative.
    // 5) Output: Provide tested, final code.

    public static BigInteger gcd(BigInteger a, BigInteger b) {
        if (a == null || b == null) {
            throw new IllegalArgumentException("Inputs must not be null");
        }
        a = a.abs();
        b = b.abs();
        if (a.equals(BigInteger.ZERO)) return b;
        if (b.equals(BigInteger.ZERO)) return a;
        return a.gcd(b);
    }

    public static void main(String[] args) {
        BigInteger[][] tests = new BigInteger[][]{
            { BigInteger.valueOf(48), BigInteger.valueOf(18) },
            { BigInteger.ZERO, BigInteger.ZERO },
            { BigInteger.valueOf(270), BigInteger.valueOf(192) },
            { BigInteger.valueOf(-24), BigInteger.valueOf(60) },
            { BigInteger.valueOf(13), BigInteger.valueOf(7) }
        };
        for (BigInteger[] t : tests) {
            BigInteger g = gcd(t[0], t[1]);
            System.out.println("gcd(" + t[0] + ", " + t[1] + ") = " + g);
        }
    }
}