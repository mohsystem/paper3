import java.util.Arrays;

public class Task1 {

    /**
     * Calculates the number of people still on the bus after the last bus stop.
     *
     * @param stops A 2D array of integer pairs. Each pair represents [people get on, people get off].
     * @return The number of people left on the bus.
     */
    public static int countPassengers(int[][] stops) {
        if (stops == null) {
            return 0;
        }
        int peopleOnBus = 0;
        for (int[] stop : stops) {
            if (stop != null && stop.length == 2) {
                peopleOnBus += stop[0];
                peopleOnBus -= stop[1];
            }
        }
        return peopleOnBus;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[][] stops1 = {{10, 0}, {3, 5}, {5, 8}};
        System.out.println("Test Case 1: " + Arrays.deepToString(stops1));
        System.out.println("Remaining passengers: " + countPassengers(stops1)); // Expected: 5

        // Test Case 2
        int[][] stops2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
        System.out.println("\nTest Case 2: " + Arrays.deepToString(stops2));
        System.out.println("Remaining passengers: " + countPassengers(stops2)); // Expected: 17

        // Test Case 3
        int[][] stops3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
        System.out.println("\nTest Case 3: " + Arrays.deepToString(stops3));
        System.out.println("Remaining passengers: " + countPassengers(stops3)); // Expected: 21

        // Test Case 4
        int[][] stops4 = {{0, 0}};
        System.out.println("\nTest Case 4: " + Arrays.deepToString(stops4));
        System.out.println("Remaining passengers: " + countPassengers(stops4)); // Expected: 0

        // Test Case 5
        int[][] stops5 = {{100, 0}, {0, 50}, {20, 30}};
        System.out.println("\nTest Case 5: " + Arrays.deepToString(stops5));
        System.out.println("Remaining passengers: " + countPassengers(stops5)); // Expected: 40
    }
}