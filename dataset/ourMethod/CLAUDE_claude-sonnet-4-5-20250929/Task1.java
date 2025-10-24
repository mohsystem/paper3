
import java.util.Arrays;

public class Task1 {
    public static int countPassengers(int[][] busStops) {
        // Validate input is not null
        if (busStops == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        
        int peopleOnBus = 0;
        
        // Process each bus stop
        for (int i = 0; i < busStops.length; i++) {
            // Validate each stop has exactly 2 elements
            if (busStops[i] == null || busStops[i].length != 2) {
                throw new IllegalArgumentException("Each bus stop must have exactly 2 values");
            }
            
            int getOn = busStops[i][0];
            int getOff = busStops[i][1];
            
            // Validate non-negative values
            if (getOn < 0 || getOff < 0) {
                throw new IllegalArgumentException("Number of people cannot be negative");
            }
            
            // Validate we don't have more people getting off than on the bus
            if (getOff > peopleOnBus) {
                throw new IllegalArgumentException("Cannot have more people getting off than on bus");
            }
            
            peopleOnBus += getOn;
            peopleOnBus -= getOff;
            
            // Ensure non-negative count
            if (peopleOnBus < 0) {
                throw new IllegalArgumentException("People count cannot be negative");
            }
        }
        
        return peopleOnBus;
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic case
        int[][] test1 = {{10, 0}, {3, 5}, {5, 8}};
        System.out.println("Test 1: " + countPassengers(test1)); // Expected: 5
        
        // Test case 2: Empty bus at end
        int[][] test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
        System.out.println("Test 2: " + countPassengers(test2)); // Expected: 17
        
        // Test case 3: Single stop
        int[][] test3 = {{5, 0}};
        System.out.println("Test 3: " + countPassengers(test3)); // Expected: 5
        
        // Test case 4: Multiple stops
        int[][] test4 = {{1, 0}, {0, 0}, {2, 1}};
        System.out.println("Test 4: " + countPassengers(test4)); // Expected: 2
        
        // Test case 5: Larger numbers
        int[][] test5 = {{100, 0}, {50, 25}, {30, 55}};
        System.out.println("Test 5: " + countPassengers(test5)); // Expected: 100
    }
}
