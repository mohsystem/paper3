import java.util.*;

public class Task5 {
    public static int persistence(long num) {
        if (num < 10) return 0;
        int count = 0;
        while (num >= 10) {
            long prod = 1;
            long n = num;
            while (n > 0) {
                prod *= (n % 10);
                n /= 10;
            }
            num = prod;
            count++;
        }
        return count;
    }

    public static void main(String[] args) {
        long[] tests = {39, 999, 4, 25, 777};
        for (long t : tests) {
            System.out.println("persistence(" + t + ") = " + persistence(t));
        }
    }
}