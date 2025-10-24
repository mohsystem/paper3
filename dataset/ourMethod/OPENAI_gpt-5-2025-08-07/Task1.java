import java.util.Arrays;

public class Task1 {
    // Returns the number of people remaining on the bus after the last stop.
    // Returns -1 for invalid input (e.g., negative numbers, wrong pair sizes, or occupancy going negative/overflow).
    public static int countPassengers(int[][] stops) {
        if (stops == null) {
            return -1;
        }
        long occupancy = 0L; // use long to guard against intermediate overflow
        for (int i = 0; i < stops.length; i++) {
            int[] pair = stops[i];
            if (pair == null || pair.length != 2) {
                return -1;
            }
            int on = pair[0];
            int off = pair[1];
            if (on < 0 || off < 0) {
                return -1;
            }
            long next = occupancy + (long) on - (long) off;
            if (next < 0) {
                return -1; // invalid: more people off than available
            }
            occupancy = next;
            if (occupancy > Integer.MAX_VALUE) {
                return -1; // prevent int overflow on return
            }
        }
        return (int) occupancy;
    }

    public static void main(String[] args) {
        int[][] t1 = new int[][]{{10, 0}, {3, 5}, {5, 8}}; // expected 5
        int[][] t2 = new int[][]{{3, 0}, {9, 1}, {4, 10}, {12, 2}}; // expected 15
        int[][] t3 = new int[][]{{0, 0}}; // expected 0
        int[][] t4 = new int[][]{{5, 0}, {0, 0}, {0, 0}}; // expected 5
        int[][] t5 = new int[][]{{5, 0}, {2, 10}}; // invalid -> expected -1

        System.out.println(countPassengers(t1));
        System.out.println(countPassengers(t2));
        System.out.println(countPassengers(t3));
        System.out.println(countPassengers(t4));
        System.out.println(countPassengers(t5));
    }
}