import java.math.BigInteger;

public class Task20 {

    public static long digPow(long n, int p) {
        if (n <= 0 || p <= 0) return -1;
        BigInteger sum = BigInteger.ZERO;
        String s = Long.toString(n);
        int exp = p;
        for (int i = 0; i < s.length(); i++) {
            int digit = s.charAt(i) - '0';
            BigInteger term = BigInteger.valueOf(digit).pow(exp);
            sum = sum.add(term);
            exp++;
        }
        BigInteger bn = BigInteger.valueOf(n);
        BigInteger[] divRem = sum.divideAndRemainder(bn);
        if (divRem[1].equals(BigInteger.ZERO)) {
            BigInteger k = divRem[0];
            if (k.compareTo(BigInteger.valueOf(Long.MAX_VALUE)) <= 0) {
                return k.longValue();
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
            {135, 1}
        };
        for (long[] t : tests) {
            long n = t[0];
            int p = (int) t[1];
            System.out.println("digPow(" + n + ", " + p + ") = " + digPow(n, p));
        }
    }
}