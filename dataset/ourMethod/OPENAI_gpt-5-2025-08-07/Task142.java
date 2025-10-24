import java.nio.charset.StandardCharsets;

public class Task142 {
    public static long gcd(long a, long b) {
        if (a == 0) {
            if (b == Long.MIN_VALUE) return Long.MIN_VALUE;
            return (b < 0) ? -b : b;
        }
        if (b == 0) {
            if (a == Long.MIN_VALUE) return Long.MIN_VALUE;
            return (a < 0) ? -a : a;
        }
        while (b != 0) {
            long r = a % b;
            a = b;
            b = r;
        }
        if (a < 0) {
            if (a == Long.MIN_VALUE) return Long.MIN_VALUE;
            return -a;
        }
        return a;
    }

    public static void main(String[] args) {
        long[][] tests = new long[][]{
            {48, 18},
            {0, 5},
            {0, 0},
            {-24, 60},
            {13, 17}
        };
        for (long[] t : tests) {
            long res = gcd(t[0], t[1]);
            System.out.println("gcd(" + t[0] + ", " + t[1] + ") = " + res);
        }
    }
}