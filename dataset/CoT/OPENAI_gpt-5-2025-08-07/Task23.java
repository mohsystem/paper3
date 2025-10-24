// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement findNb(m) that returns n if sum_{k=1..n} k^3 = m, else -1.
// 2) Security requirements: Avoid overflows by using BigInteger, validate inputs.
// 3) Secure coding generation: Use monotonic BigInteger binary search to compare f(n) to m.
// 4) Code review: All math uses safe BigInteger, no unchecked casts, handle negative and zero.
// 5) Secure code output: Final code below with tests.
import java.math.BigInteger;

public class Task23 {
    public static long findNb(long m) {
        if (m < 0) return -1;
        if (m == 0) return 0;
        BigInteger M = BigInteger.valueOf(m);
        long low = 0L, high = 1L;
        while (compareF(high, M) < 0 && high < Long.MAX_VALUE / 2) {
            high <<= 1;
        }
        long ans = -1L;
        while (low <= high) {
            long mid = low + ((high - low) >>> 1);
            int cmp = compareF(mid, M);
            if (cmp == 0) {
                ans = mid;
                break;
            } else if (cmp < 0) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return ans;
    }

    private static int compareF(long n, BigInteger M) {
        return f(n).compareTo(M);
    }

    private static BigInteger f(long n) {
        BigInteger bn = BigInteger.valueOf(n);
        BigInteger t = bn.multiply(bn.add(BigInteger.ONE)).shiftRight(1); // (n*(n+1))/2
        return t.multiply(t);
    }

    public static void main(String[] args) {
        long[] tests = new long[] {
            1071225L,            // -> 45
            91716553919377L,     // -> -1
            4183059834009L,      // -> 2022
            24723578342962L,     // -> -1
            135440716410000L     // -> 4824
        };
        for (long m : tests) {
            System.out.println(findNb(m));
        }
    }
}