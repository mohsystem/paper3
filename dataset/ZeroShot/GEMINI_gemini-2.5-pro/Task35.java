import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Task35 {

    /**
     * Calculates the number of people still on the bus after the last bus stop.
     *
     * @param stops A list of integer arrays. Each array represents a bus stop
     *              with two elements: [people_getting_on, people_getting_off].
     * @return The number of people remaining on the bus.
     */
    public static int countPassengers(List<int[]> stops) {
        int peopleOnBus = 0;
        if (stops == null) {
            return 0;
        }
        for (int[] stop : stops) {
            // Assuming input is always valid with pairs of [on, off]
            peopleOnBus += stop[0];
            peopleOnBus -= stop[1];
        }
        return peopleOnBus;
    }

    public static void main(String[] args) {
        // Test Case 1
        System.out.println(countPassengers(new ArrayList<>(Arrays.asList(new int[]{10, 0}, new int[]{3, 5}, new int[]{5, 8}))));

        // Test Case 2
        System.out.println(countPassengers(new ArrayList<>(Arrays.asList(new int[]{3, 0}, new int[]{9, 1}, new int[]{4, 10}, new int[]{12, 2}, new int[]{6, 1}, new int[]{7, 10}))));

        // Test Case 3: Single stop with no change
        System.out.println(countPassengers(new ArrayList<>(Arrays.asList(new int[]{0, 0}))));

        // Test Case 4: Ends with zero passengers
        System.out.println(countPassengers(new ArrayList<>(Arrays.asList(new int[]{100, 0}, new int[]{0, 50}, new int[]{25, 0}, new int[]{0, 75}))));
        
        // Test Case 5: Net change is zero at most stops
        System.out.println(countPassengers(new ArrayList<>(Arrays.asList(new int[]{5, 0}, new int[]{5, 5}, new int[]{5, 5}, new int[]{5, 5}, new int[]{5, 5}))));
    }
}