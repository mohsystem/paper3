public class Task5 {
    public static int persistence(long num) {
        if (num < 0) {
            throw new IllegalArgumentException("num must be non-negative");
        }
        int steps = 0;
        while (num >= 10) {
            long prod = 1;
            long t = num;
            while (t > 0) {
                prod *= (t % 10);
                t /= 10;
            }
            num = prod;
            steps++;
        }
        return steps;
    }

    public static void main(String[] args) {
        long[] tests = {39L, 999L, 4L, 25L, 77L};
        for (long n : tests) {
            System.out.println("persistence(" + n + ") = " + persistence(n));
        }
    }
}