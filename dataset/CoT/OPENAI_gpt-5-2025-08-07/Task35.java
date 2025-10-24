// Chain-of-Through process in code generation:
// 1) Problem understanding: compute remaining people on a bus given pairs [on, off].
// 2) Security requirements: validate inputs, avoid negative totals, and guard against overflow.
// 3) Secure coding generation: use safe arithmetic (Math.addExact/subtractExact), validate all pairs.
// 4) Code review: ensure checks for nulls, lengths, and values; handle empty input safely.
// 5) Secure code output: final code with tests demonstrating correct functionality.

import java.util.Arrays;

public class Task35 {

    // Computes remaining people on the bus after processing all stops.
    // Each stop is a pair [on, off]. Returns total remaining.
    public static long peopleOnBus(long[][] stops) {
        long total = 0L;
        if (stops == null) {
            return 0L;
        }
        for (int i = 0; i < stops.length; i++) {
            long on = 0L;
            long off = 0L;
            if (stops[i] != null && stops[i].length >= 2) {
                on = stops[i][0];
                off = stops[i][1];
            }
            if (on < 0 || off < 0) {
                throw new IllegalArgumentException("Negative values are not allowed.");
            }
            long temp;
            try {
                temp = Math.addExact(total, on);
                temp = Math.subtractExact(temp, off);
            } catch (ArithmeticException ex) {
                throw new IllegalArgumentException("Overflow detected during computation.", ex);
            }
            if (temp < 0) {
                throw new IllegalArgumentException("Bus population would become negative.");
            }
            total = temp;
        }
        return total;
    }

    // 5 test cases
    public static void main(String[] args) {
        long[][] t1 = { {10, 0}, {3, 5}, {5, 8} }; // expected 5
        long[][] t2 = { {3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10} }; // expected 17
        long[][] t3 = {}; // expected 0
        long[][] t4 = { {0, 0} }; // expected 0
        long[][] t5 = {
            {1_000_000_000_000L, 0L},
            {2_000_000_000_000L, 500_000_000_000L},
            {3_000_000_000_000L, 1_000_000_000_000L}
        }; // expected 4_500_000_000_000

        long[] results = new long[] {
            peopleOnBus(t1),
            peopleOnBus(t2),
            peopleOnBus(t3),
            peopleOnBus(t4),
            peopleOnBus(t5)
        };

        for (int i = 0; i < results.length; i++) {
            System.out.println("Test " + (i + 1) + ": " + results[i]);
        }
    }
}