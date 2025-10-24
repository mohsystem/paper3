import java.util.*;

public class Task34 {
    public static boolean narcissistic(long n) {
        if (n <= 0) return false; // per problem, inputs are positive non-zero
        int digits = countDigits(n);
        long sum = 0;
        long temp = n;
        while (temp > 0) {
            int d = (int)(temp % 10);
            sum += ipow(d, digits);
            temp /= 10;
        }
        return sum == n;
    }

    private static int countDigits(long n) {
        int c = 0;
        while (n > 0) {
            c++;
            n /= 10;
        }
        return c;
    }

    private static long ipow(int base, int exp) {
        long result = 1;
        long b = base;
        int e = exp;
        while (e > 0) {
            if ((e & 1) == 1) result *= b;
            b *= b;
            e >>= 1;
        }
        return result;
    }

    public static void main(String[] args) {
        long[] tests = {153, 1652, 7, 370, 9474};
        for (long t : tests) {
            System.out.println(t + " -> " + narcissistic(t));
        }
    }
}