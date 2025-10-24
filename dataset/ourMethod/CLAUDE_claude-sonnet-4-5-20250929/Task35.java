
import java.util.Arrays;

public class Task35 {
    public static int countPassengers(int[][] busStops) {
        // Validate input
        if (busStops == null) {
            throw new IllegalArgumentException("Input array cannot be null");
        }
        
        int totalPeople = 0;
        
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
            
            // Validate first stop (no one gets off)
            if (i == 0 && getOff != 0) {
                throw new IllegalArgumentException("No one can get off at the first stop");
            }
            
            totalPeople += getOn;
            totalPeople -= getOff;
            
            // Validate people count is never negative
            if (totalPeople < 0) {
                throw new IllegalArgumentException("Number of people on bus cannot be negative");
            }
        }
        
        return totalPeople;
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple case
        int[][] test1 = {{10, 0}, {3, 5}, {5, 8}};
        System.out.println("Test 1: " + countPassengers(test1)); // Expected: 5
        
        // Test case 2: Empty bus at the end
        int[][] test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
        System.out.println("Test 2: " + countPassengers(test2)); // Expected: 17
        
        // Test case 3: Single stop
        int[][] test3 = {{5, 0}};
        System.out.println("Test 3: " + countPassengers(test3)); // Expected: 5
        
        // Test case 4: Multiple stops
        int[][] test4 = {{10, 0}, {3, 5}, {2, 5}};
        System.out.println("Test 4: " + countPassengers(test4)); // Expected: 5
        
        // Test case 5: All get off at last stop
        int[][] test5 = {{20, 0}, {5, 3}, {10, 8}, {0, 24}};
        System.out.println("Test 5: " + countPassengers(test5)); // Expected: 0
    }
}
