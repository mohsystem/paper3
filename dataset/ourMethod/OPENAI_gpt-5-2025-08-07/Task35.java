import java.util.Arrays;

public class Task35 {

    // Returns final number of passengers or -1 on invalid input.
    public static int countPassengers(int[][] stops) {
        if (stops == null) {
            return 0; // Treat null as empty input
        }
        long current = 0L;
        for (int i = 0; i < stops.length; i++) {
            int[] pair = stops[i];
            if (pair == null || pair.length < 2) {
                return -1;
            }
            int on = pair[0];
            int off = pair[1];
            if (on < 0 || off < 0) {
                return -1;
            }
            // Check for overflow on addition
            if (current > (long) Integer.MAX_VALUE - (long) on) {
                return -1;
            }
            current += on;
            // Validate that we never drop below zero and no overflow on subtraction
            if (off > current) {
                return -1;
            }
            current -= off;
        }
        if (current < 0 || current > Integer.MAX_VALUE) {
            return -1;
        }
        return (int) current;
    }

    public static void main(String[] args) {
        int[][] tc1 = new int[][] { {10,0}, {3,5}, {5,8} }; // expected 5
        int[][] tc2 = new int[][] { {3,0}, {9,1}, {4,10}, {12,2}, {6,1}, {7,10} }; // expected 17
        int[][] tc3 = new int[][] { {0,0} }; // expected 0
        int[][] tc4 = new int[][] { {5,0}, {0,0}, {0,0} }; // expected 5
        int[][] tc5 = new int[][] { {1_000_000,0}, {0,999_999}, {0,0}, {0,1} }; // expected 0

        System.out.println(countPassengers(tc1));
        System.out.println(countPassengers(tc2));
        System.out.println(countPassengers(tc3));
        System.out.println(countPassengers(tc4));
        System.out.println(countPassengers(tc5));
    }
}