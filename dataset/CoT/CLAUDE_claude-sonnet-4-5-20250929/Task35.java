
public class Task35 {
    public static int countPassengers(int[][] busStops) {
        // Input validation
        if (busStops == null) {
            return 0;
        }
        
        int peopleOnBus = 0;
        
        // Process each bus stop
        for (int i = 0; i < busStops.length; i++) {
            // Validate array structure
            if (busStops[i] == null || busStops[i].length != 2) {
                continue;
            }
            
            // Validate non-negative values
            if (busStops[i][0] < 0 || busStops[i][1] < 0) {
                continue;
            }
            
            // Add people getting on, subtract people getting off
            peopleOnBus += busStops[i][0];
            peopleOnBus -= busStops[i][1];
            
            // Ensure non-negative passenger count
            if (peopleOnBus < 0) {
                peopleOnBus = 0;
            }
        }
        
        return peopleOnBus;
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple case
        int[][] test1 = {{10, 0}, {3, 5}, {5, 8}};
        System.out.println("Test 1: " + countPassengers(test1)); // Expected: 5
        
        // Test case 2: All get off
        int[][] test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
        System.out.println("Test 2: " + countPassengers(test2)); // Expected: 17
        
        // Test case 3: Empty bus at end
        int[][] test3 = {{5, 0}, {2, 3}, {0, 4}};
        System.out.println("Test 3: " + countPassengers(test3)); // Expected: 0
        
        // Test case 4: Single stop
        int[][] test4 = {{10, 0}};
        System.out.println("Test 4: " + countPassengers(test4)); // Expected: 10
        
        // Test case 5: Multiple stops
        int[][] test5 = {{3, 0}, {2, 1}, {5, 3}, {1, 4}};
        System.out.println("Test 5: " + countPassengers(test5)); // Expected: 3
    }
}
