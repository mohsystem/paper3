// Chain-of-Through Process:
// 1) Problem understanding: compute remaining people on the bus after processing pairs of (on, off).
// 2) Security requirements: validate inputs (non-null, non-negative, consistent transitions), avoid overflow.
// 3) Secure coding generation: use long for accumulation, validate each stop.
// 4) Code review: ensure no negative totals, handle invalid formats.
// 5) Secure code output: final method returns non-negative count or throws clear exceptions if invalid.
public class Task1 {
    public static int countPassengers(int[][] stops) {
        if (stops == null) return 0;
        long total = 0L;
        for (int i = 0; i < stops.length; i++) {
            if (stops[i] == null || stops[i].length < 2) {
                throw new IllegalArgumentException("Invalid stop format at index " + i);
            }
            int on = stops[i][0];
            int off = stops[i][1];
            if (on < 0 || off < 0) {
                throw new IllegalArgumentException("Negative values are not allowed at index " + i);
            }
            long available = total + on;
            if (off > available) {
                throw new IllegalArgumentException("More people getting off than available at index " + i);
            }
            total = available - off;
            if (total < 0) {
                throw new IllegalStateException("Total passengers became negative unexpectedly at index " + i);
            }
            if (total > Integer.MAX_VALUE) {
                throw new ArithmeticException("Passenger count exceeds allowed range");
            }
        }
        return (int) total;
    }

    // 5 test cases
    public static void main(String[] args) {
        int[][] t1 = { {10, 0}, {3, 5}, {5, 8} }; // expected 5
        int[][] t2 = { }; // expected 0
        int[][] t3 = { {0, 0} }; // expected 0
        int[][] t4 = { {3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10} }; // expected 17
        int[][] t5 = { {5, 0}, {0, 5} }; // expected 0

        System.out.println(countPassengers(t1));
        System.out.println(countPassengers(t2));
        System.out.println(countPassengers(t3));
        System.out.println(countPassengers(t4));
        System.out.println(countPassengers(t5));
    }
}