import java.io.*;

public class Task10 {
    public static long getSum(long a, long b) {
        long lo = Math.min(a, b);
        long hi = Math.max(a, b);
        long n = hi - lo + 1;
        if ((n & 1L) == 0L) {
            return (n / 2) * (lo + hi);
        } else {
            return n * ((lo + hi) / 2);
        }
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