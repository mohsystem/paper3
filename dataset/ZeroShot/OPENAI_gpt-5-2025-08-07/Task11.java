import java.util.*;

public class Task11 {
    public static int nb_year(long p0, double percent, long aug, long p) {
        if (p0 <= 0 || p <= 0 || percent < 0.0 || p0 == Long.MIN_VALUE || p == Long.MIN_VALUE) {
            return -1;
        }
        if (p0 >= p) {
            return 0;
        }
        if (percent <= 0.0 && aug <= 0) {
            return -1;
        }
        long pop = p0;
        int years = 0;
        final int MAX_YEARS = 10000000;
        while (pop < p && years < MAX_YEARS) {
            double growthD = (pop * (percent / 100.0));
            long growth;
            if (Double.isNaN(growthD) || Double.isInfinite(growthD)) {
                growth = Long.MAX_VALUE;
            } else if (growthD >= Long.MAX_VALUE) {
                growth = Long.MAX_VALUE;
            } else if (growthD <= Long.MIN_VALUE) {
                growth = Long.MIN_VALUE;
            } else {
                growth = (long) Math.floor(growthD);
            }
            long next = safeAdd(pop, growth);
            next = safeAdd(next, aug);
            if (next == pop) {
                // No change; cannot progress further
                return -1;
            }
            pop = next;
            years++;
        }
        return pop >= p ? years : -1;
    }

    private static long safeAdd(long a, long b) {
        if (b > 0 && a > Long.MAX_VALUE - b) return Long.MAX_VALUE;
        if (b < 0 && a < Long.MIN_VALUE - b) return Long.MIN_VALUE;
        return a + b;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(nb_year(1000, 2, 50, 1200));       // expected 3
        System.out.println(nb_year(1500, 5, 100, 5000));      // expected 15
        System.out.println(nb_year(1500000, 2.5, 10000, 2000000)); // expected 10
        System.out.println(nb_year(1000, 0, -1, 1200));       // expected -1 (cannot reach)
        System.out.println(nb_year(5000, 2, 100, 3000));      // expected 0 (already reached)
    }
}