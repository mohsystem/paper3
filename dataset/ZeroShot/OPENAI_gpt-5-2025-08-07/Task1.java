import java.util.Objects;

public class Task1 {

    public static long peopleOnBus(int[][] stops) {
        if (stops == null) {
            throw new IllegalArgumentException("stops cannot be null");
        }
        long current = 0L;
        for (int i = 0; i < stops.length; i++) {
            int[] pair = stops[i];
            if (pair == null || pair.length != 2) {
                throw new IllegalArgumentException("Each stop must be a pair of two integers");
            }
            int on = pair[0];
            int off = pair[1];
            if (on < 0 || off < 0) {
                throw new IllegalArgumentException("Counts cannot be negative");
            }
            if (i == 0 && off != 0) {
                throw new IllegalArgumentException("First stop must have 0 getting off");
            }
            long capacityBeforeOff = current + (long) on;
            if (off > capacityBeforeOff) {
                throw new IllegalArgumentException("More people getting off than available on the bus");
            }
            current = capacityBeforeOff - off;
        }
        if (current < 0) {
            throw new IllegalStateException("Resulting count cannot be negative");
        }
        return current;
    }

    public static void main(String[] args) {
        int[][] case1 = new int[][]{{10, 0}, {3, 5}, {5, 8}};                      // Expected: 5
        int[][] case2 = new int[][]{{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}}; // Expected: 17
        int[][] case3 = new int[][]{{0, 0}};                                        // Expected: 0
        int[][] case4 = new int[][]{{20, 0}, {0, 10}, {5, 5}};                      // Expected: 10
        int[][] case5 = new int[][]{{5, 0}, {0, 5}, {5, 0}, {0, 5}};                // Expected: 0

        System.out.println(peopleOnBus(case1));
        System.out.println(peopleOnBus(case2));
        System.out.println(peopleOnBus(case3));
        System.out.println(peopleOnBus(case4));
        System.out.println(peopleOnBus(case5));
    }
}