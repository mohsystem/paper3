import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task1 {

    /**
     * Calculates the number of people remaining on the bus after all stops.
     *
     * @param stops A list of integer arrays. Each array represents a stop
     *              with two elements: [people_getting_on, people_getting_off].
     * @return The number of people on the bus after the last stop.
     */
    public static int countPassengers(List<int[]> stops) {
        // The problem statement ensures the number of people never goes below zero.
        // A check for null or empty list is good practice.
        if (stops == null || stops.isEmpty()) {
            return 0;
        }

        int peopleOnBus = 0;
        for (int[] stop : stops) {
            // Assuming each stop array has 2 elements as per the problem description.
            peopleOnBus += stop[0]; // People getting on
            peopleOnBus -= stop[1]; // People getting off
        }
        return peopleOnBus;
    }

    public static void main(String[] args) {
        // Test Case 1
        List<int[]> test1 = new ArrayList<>(Arrays.asList(new int[]{10, 0}, new int[]{3, 5}, new int[]{5, 8}));
        System.out.println("Test Case 1: " + countPassengers(test1)); // Expected: 5

        // Test Case 2
        List<int[]> test2 = new ArrayList<>(Arrays.asList(new int[]{3, 0}, new int[]{9, 1}, new int[]{4, 10}, new int[]{12, 2}, new int[]{6, 1}, new int[]{7, 10}));
        System.out.println("Test Case 2: " + countPassengers(test2)); // Expected: 17

        // Test Case 3
        List<int[]> test3 = new ArrayList<>(Arrays.asList(new int[]{3, 0}, new int[]{9, 1}, new int[]{4, 8}, new int[]{12, 2}, new int[]{6, 1}, new int[]{7, 8}));
        System.out.println("Test Case 3: " + countPassengers(test3)); // Expected: 21

        // Test Case 4: A single stop with no activity
        List<int[]> test4 = new ArrayList<>(Arrays.asList(new int[]{0, 0}));
        System.out.println("Test Case 4: " + countPassengers(test4)); // Expected: 0

        // Test Case 5: Empty list of stops
        List<int[]> test5 = new ArrayList<>();
        System.out.println("Test Case 5: " + countPassengers(test5)); // Expected: 0
    }
}