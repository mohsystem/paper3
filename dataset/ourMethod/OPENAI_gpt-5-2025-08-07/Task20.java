import java.math.BigInteger;

public class Task20 {

    public static long digPow(long n, int p) {
        if (n <= 0 || p <= 0) {
            return -1L;
        }
        String s = Long.toString(n);
        BigInteger sum = BigInteger.ZERO;
        int exp = p;
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch < '0' || ch > '9') {
                return -1L;
            }
            int d = ch - '0';
            sum = sum.add(BigInteger.valueOf(d).pow(exp));
            exp++;
        }
        BigInteger bn = BigInteger.valueOf(n);
        BigInteger[] divRem = sum.divideAndRemainder(bn);
        if (divRem[1].signum() == 0) {
            BigInteger k = divRem[0];
            if (k.bitLength() <= 63) {
                return k.longValue();
            } else {
                return -1L;
            }
        } else {
            return -1L;
        }
    }

    public static void main(String[] args) {
        long[][] tests = {
            {89L, 1L},
            {92L, 1L},
            {695L, 2L},
            {46288L, 3L},
            {1L, 1L}
        };
        for (long[] t : tests) {
            long n = t[0];
            int p = (int) t[1];
            long res = digPow(n, p);
            System.out.println("digPow(" + n + ", " + p + ") = " + res);
        }
    }
}