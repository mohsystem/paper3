import java.math.BigInteger;

public class Task20 {
    public static long digPow(long n, int p) {
        String s = Long.toString(n);
        BigInteger sum = BigInteger.ZERO;
        int exp = p;
        for (int i = 0; i < s.length(); i++) {
            int d = s.charAt(i) - '0';
            sum = sum.add(BigInteger.valueOf(d).pow(exp));
            exp++;
        }
        BigInteger bn = BigInteger.valueOf(n);
        BigInteger[] divRem = sum.divideAndRemainder(bn);
        if (divRem[1].equals(BigInteger.ZERO)) {
            return divRem[0].longValue();
        }
        return -1;
    }

    public static void main(String[] args) {
        System.out.println(digPow(89, 1));     // 1
        System.out.println(digPow(92, 1));     // -1
        System.out.println(digPow(695, 2));    // 2
        System.out.println(digPow(46288, 3));  // 51
        System.out.println(digPow(135, 1));    // 1
    }
}