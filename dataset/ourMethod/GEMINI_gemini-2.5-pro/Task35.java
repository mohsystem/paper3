import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task35 {

    /**
     * Calculates the number of people still on the bus after the last bus stop.
     *
     * @param stops A list of integer pairs. Each pair represents [people_on, people_off].
     * @return The number of people remaining on the bus.
     */
    public static int countPassengers(List<int[]> stops) {
        int peopleOnBus = 0;
        if (stops == null) {
            return 0;
        }
        for (int[] stop : stops) {
            // Assuming each stop array is valid with at least 2 elements as per problem description
            peopleOnBus += stop[0];
            peopleOnBus -= stop[1];
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
        
        // Test Case 4
        List<int[]> test4 = new ArrayList<>(Arrays.asList(new int[]{1, 0}));
        System.out.println("Test Case 4: " + countPassengers(test4)); // Expected: 1

        // Test Case 5
        List<int[]> test5 = new ArrayList<>();
        System.out.println("Test Case 5 (Empty): " + countPassengers(test5)); // Expected: 0
    }
}