import java.math.BigInteger;

public class Task20 {
    public static long digPow(long n, int p) {
        if (n <= 0 || p <= 0) return -1;
        String s = Long.toString(n);
        BigInteger sum = BigInteger.ZERO;
        int exp = p;
        for (int i = 0; i < s.length(); i++, exp++) {
            int digit = s.charAt(i) - '0';
            sum = sum.add(BigInteger.valueOf(digit).pow(exp));
        }
        BigInteger nBI = BigInteger.valueOf(n);
        BigInteger[] qr = sum.divideAndRemainder(nBI);
        if (qr[1].equals(BigInteger.ZERO)) {
            try {
                long k = qr[0].longValueExact();
                return k > 0 ? k : -1;
            } catch (ArithmeticException ex) {
                return -1;
            }
        }
        return -1;
    }

    public static void main(String[] args) {
        long[][] tests = {
            {89, 1},
            {92, 1},
            {695, 2},
            {46288, 3},
            {1, 1}
        };
        for (long[] t : tests) {
            long n = t[0];
            int p = (int) t[1];
            System.out.println("digPow(" + n + ", " + p + ") = " + digPow(n, p));
        }
    }
}