public class Task142 {
    public static long gcd(long a, long b) {
        a = Math.abs(a);
        b = Math.abs(b);
        if (a == 0) return b;
        if (b == 0) return a;
        while (b != 0) {
            long t = a % b;
            a = b;
            b = t;
        }
        return a;
    }

    public static void main(String[] args) {
        long[][] tests = {
            {54, 24},
            {0, 5},
            {17, 13},
            {-48, 18},
            {0, 0}
        };
        for (long[] t : tests) {
            long res = gcd(t[0], t[1]);
            System.out.println("gcd(" + t[0] + ", " + t[1] + ") = " + res);
        }
    }
}