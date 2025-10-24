import java.util.Arrays;

public class Task23 {
    public static long findNb(long m) {
        if (m <= 0) return -1;
        long k = isPerfectSquare(m);
        if (k < 0) return -1;

        long d = 1 + 8L * k;
        long sd = isPerfectSquare(d);
        if (sd < 0) return -1;

        long n = (sd - 1) / 2;
        long t;
        if ((n & 1L) == 0L) {
            t = (n / 2L) * (n + 1L);
        } else {
            t = n * ((n + 1L) / 2L);
        }
        return (t == k) ? n : -1;
    }

    private static long isPerfectSquare(long x) {
        if (x < 0) return -1;
        long low = 0, high = Math.min(x, 3037000499L);
        while (low <= high) {
            long mid = low + ((high - low) >>> 1);
            if (mid == 0 || mid <= x / mid) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        long r = high;
        return (r >= 0 && r * r == x) ? r : -1;
    }

    public static void main(String[] args) {
        long[] tests = {
            1071225L,          // -> 45
            91716553919377L,   // -> -1
            4183059834009L,    // -> 2022
            1L,                // -> 1
            0L                 // -> -1
        };
        for (long m : tests) {
            System.out.println("findNb(" + m + ") = " + findNb(m));
        }
    }
}