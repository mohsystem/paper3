import java.math.BigInteger;

public class Task57 {
    public static BigInteger factorial(long n) {
        if (n < 0) {
            throw new IllegalArgumentException("Input must be non-negative.");
        }
        BigInteger result = BigInteger.ONE;
        for (long i = 2; i <= n; i++) {
            result = result.multiply(BigInteger.valueOf(i));
        }
        return result;
    }

    public static void main(String[] args) {
        long[] testCases = {0, 1, 5, 10, 20};
        for (long n : testCases) {
            try {
                BigInteger f = factorial(n);
                System.out.println("factorial(" + n + ") = " + f.toString());
            } catch (IllegalArgumentException e) {
                System.out.println("factorial(" + n + ") error: " + e.getMessage());
            }
        }
    }
}