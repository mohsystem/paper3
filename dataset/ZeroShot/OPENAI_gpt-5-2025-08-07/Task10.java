import java.math.BigInteger;

public class Task10 {
    public static BigInteger getSum(long a, long b) {
        if (a == b) {
            return BigInteger.valueOf(a);
        }
        long lo = Math.min(a, b);
        long hi = Math.max(a, b);

        BigInteger n = BigInteger.valueOf(hi).subtract(BigInteger.valueOf(lo)).add(BigInteger.ONE);
        BigInteger sumEnds = BigInteger.valueOf(lo).add(BigInteger.valueOf(hi));

        if (!n.testBit(0)) { // n is even
            n = n.shiftRight(1);
        } else {
            sumEnds = sumEnds.shiftRight(1);
        }

        return n.multiply(sumEnds);
    }

    public static void main(String[] args) {
        long[][] tests = {
            {1, 0},
            {1, 2},
            {0, 1},
            {1, 1},
            {-1, 2}
        };
        for (long[] t : tests) {
            System.out.println(getSum(t[0], t[1]));
        }
    }
}