import java.nio.charset.StandardCharsets;

public class Task11 {

    public static int nb_year(long p0, double percent, long aug, long p) {
        if (p0 <= 0 || p <= 0 || percent < 0.0 || Double.isNaN(percent) || Double.isInfinite(percent)) {
            return -1;
        }
        if (p0 >= p) {
            return 0;
        }
        if (percent == 0.0 && aug <= 0) {
            return -1;
        }

        long current = p0;
        int years = 0;
        final int MAX_YEARS = 10_000_000;

        while (current < p) {
            if (years >= MAX_YEARS) {
                return -1;
            }
            long growth = (long) Math.floor(current * (percent / 100.0));
            long next = current + growth + aug;
            if (next < 0) {
                next = 0;
            }
            if (next == current) {
                return -1; // Stuck, cannot progress further
            }
            current = next;
            years++;
        }
        return years;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(nb_year(1000L, 2.0, 50L, 1200L));       // Expected 3
        System.out.println(nb_year(1500L, 5.0, 100L, 5000L));      // Expected 15
        System.out.println(nb_year(1500000L, 2.5, 10000L, 2000000L)); // Expected 10
        System.out.println(nb_year(1000L, 0.0, 0L, 1000L));        // Expected 0
        System.out.println(nb_year(1000L, 0.0, -1L, 1200L));       // Expected -1 (unreachable)
    }
}