import java.util.ArrayList;
import java.util.Arrays;

class Task35 {
    public static int countPassengers(ArrayList<int[]> stops) {
        int passengers = 0;
        for (int[] stop : stops) {
            passengers += stop[0];
            passengers -= stop[1];
        }
        return passengers;
    }

    public static void main(String[] args) {
        // Test Case 1
        ArrayList<int[]> test1 = new ArrayList<>(Arrays.asList(new int[]{10, 0}, new int[]{3, 5}, new int[]{5, 8}));
        System.out.println("Remaining passengers: " + countPassengers(test1)); // Expected: 5

        // Test Case 2
        ArrayList<int[]> test2 = new ArrayList<>(Arrays.asList(new int[]{3, 0}, new int[]{9, 1}, new int[]{4, 10}, new int[]{12, 2}, new int[]{6, 1}, new int[]{7, 10}));
        System.out.println("Remaining passengers: " + countPassengers(test2)); // Expected: 17

        // Test Case 3
        ArrayList<int[]> test3 = new ArrayList<>(Arrays.asList(new int[]{3, 0}, new int[]{9, 1}, new int[]{4, 8}, new int[]{12, 2}, new int[]{6, 1}, new int[]{7, 8}));
        System.out.println("Remaining passengers: " + countPassengers(test3)); // Expected: 21

        // Test Case 4
        ArrayList<int[]> test4 = new ArrayList<>(Arrays.asList(new int[]{0, 0}));
        System.out.println("Remaining passengers: " + countPassengers(test4)); // Expected: 0
        
        // Test Case 5
        ArrayList<int[]> test5 = new ArrayList<>(Arrays.asList(new int[]{100, 0}, new int[]{20, 30}, new int[]{5, 15}));
        System.out.println("Remaining passengers: " + countPassengers(test5)); // Expected: 80
    }
}