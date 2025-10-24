import java.util.Arrays;

public class Task5 {

    // Returns multiplicative persistence of a non-negative number.
    // Returns -1 for invalid (negative) input.
    public static int persistence(long num) {
        if (num < 0L) {
            return -1; // invalid input
        }
        int count = 0;
        while (num >= 10L) {
            long prod = 1L;
            long n = num;
            while (n > 0L) {
                prod *= (n % 10L);
                n /= 10L;
            }
            num = prod;
            count++;
        }
        return count;
    }

    private static void runTest(long num) {
        int result = persistence(num);
        System.out.println("persistence(" + num + ") = " + result);
    }

    public static void main(String[] args) {
        long[] tests = new long[] {39L, 999L, 4L, 25L, 444L};
        for (long t : tests) {
            runTest(t);
        }
    }
}