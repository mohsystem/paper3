import java.math.BigInteger;

public class Task57 {
    public static BigInteger factorial(int n) {
        if (n < 0) throw new IllegalArgumentException("Negative input not allowed");
        BigInteger result = BigInteger.ONE;
        for (int i = 2; i <= n; i++) {
            result = result.multiply(BigInteger.valueOf(i));
        }
        return result;
    }

    public static void main(String[] args) {
        int[] tests = {0, 1, 5, 10, 20};
        for (int t : tests) {
            System.out.println(t + "! = " + factorial(t));
        }
    }
}