import java.nio.charset.StandardCharsets;

public class Task23 {
    public static void main(String[] args) {
        long[] tests = new long[] {
            1071225L,          // expected 45
            91716553919377L,   // expected -1
            1L,                // expected 1
            4183059834009L,    // expected 2022
            0L                 // expected 0
        };
        for (long m : tests) {
            long n = findNb(m);
            System.out.println(n);
        }
    }

    public static long findNb(long m) {
        if (m < 0) return -1;
        if (m == 0) return 0;

        long s = isqrt(m);
        if (s < 0 || s * s != m) return -1;

        long disc = 1 + 8L * s;
        long r = isqrt(disc);
        if (r < 0 || r * r != disc) return -1;

        long n = (r - 1) / 2;
        if (triFromN(n) == s) return n;
        return -1;
    }

    private static long triFromN(long n) {
        if ((n & 1L) == 0L) {
            return (n / 2L) * (n + 1L);
        } else {
            return n * ((n + 1L) / 2L);
        }
    }

    private static long isqrt(long x) {
        if (x < 0) return -1;
        long lo = 0;
        long hi = Math.min(x, 3037000500L); // floor(sqrt(Long.MAX_VALUE)) = 3037000499
        long ans = 0;
        while (lo <= hi) {
            long mid = lo + ((hi - lo) >>> 1);
            if (mid == 0) {
                ans = 0;
                lo = 1;
                continue;
            }
            if (mid <= x / mid) {
                ans = mid;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        return ans;
    }
}