import java.math.BigInteger;

public class Task5 {
    public static int persistence(BigInteger num) {
        if (num == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        if (num.compareTo(BigInteger.ZERO) < 0) {
            throw new IllegalArgumentException("Input must be non-negative");
        }
        if (num.compareTo(BigInteger.TEN) < 0) {
            return 0;
        }
        int count = 0;
        while (num.compareTo(BigInteger.TEN) >= 0) {
            String s = num.toString();
            BigInteger prod = BigInteger.ONE;
            for (int i = 0; i < s.length(); i++) {
                int d = s.charAt(i) - '0';
                if (d == 0) {
                    prod = BigInteger.ZERO;
                    break;
                }
                prod = prod.multiply(BigInteger.valueOf(d));
            }
            num = prod;
            count++;
        }
        return count;
    }

    public static int persistence(long num) {
        if (num < 0) throw new IllegalArgumentException("Input must be non-negative");
        return persistence(BigInteger.valueOf(num));
    }

    public static void main(String[] args) {
        long[] tests = {39, 999, 4, 25, 77};
        for (long t : tests) {
            System.out.println("persistence(" + t + ") = " + persistence(t));
        }
    }
}