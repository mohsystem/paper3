import java.util.Arrays;

public class Task35 {
    public static long countPassengers(int[][] stops) {
        if (stops == null) {
            throw new IllegalArgumentException("stops cannot be null");
        }
        long count = 0L;
        for (int i = 0; i < stops.length; i++) {
            int[] pair = stops[i];
            if (pair == null || pair.length < 2) {
                throw new IllegalArgumentException("Each stop must be a pair of two non-negative integers");
            }
            int onRaw = pair[0];
            int offRaw = pair[1];
            if (onRaw < 0 || offRaw < 0) {
                throw new IllegalArgumentException("Values must be non-negative");
            }
            long on = onRaw;
            long off = offRaw;

            if (i == 0 && off != 0L) {
                throw new IllegalArgumentException("At first stop, number of people getting off must be zero");
            }

            if (on > Long.MAX_VALUE - count) {
                throw new ArithmeticException("Overflow while adding passengers");
            }
            long afterBoard = count + on;
            if (off > afterBoard) {
                throw new IllegalArgumentException("More people getting off than currently on the bus");
            }
            count = afterBoard - off;
        }
        return count;
    }

    private static void runTest(int[][] stops) {
        try {
            long result = countPassengers(stops);
            System.out.println("Stops: " + Arrays.deepToString(stops) + " -> Remaining: " + result);
        } catch (Exception e) {
            System.out.println("Stops: " + Arrays.deepToString(stops) + " -> Error: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        int[][] t1 = new int[][]{{10, 0}, {3, 5}, {5, 8}};
        int[][] t2 = new int[][]{{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
        int[][] t3 = new int[][]{{0, 0}};
        int[][] t4 = new int[][]{{5, 0}, {0, 0}, {0, 0}};
        int[][] t5 = new int[][]{{1_000_000_000, 0}, {1_000_000_000, 0}, {0, 1_000_000_000}};

        runTest(t1);
        runTest(t2);
        runTest(t3);
        runTest(t4);
        runTest(t5);
    }
}