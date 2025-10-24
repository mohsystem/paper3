
public class Task1 {
    public static int countPassengers(int[][] busStops) {
        // Input validation
        if (busStops == null || busStops.length == 0) {
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
            int getOn = Math.max(0, busStops[i][0]);
            int getOff = Math.max(0, busStops[i][1]);
            
            // Update passenger count
            peopleOnBus += getOn;
            peopleOnBus -= getOff;
            
            // Ensure non-negative result
            peopleOnBus = Math.max(0, peopleOnBus);
        }
        
        return peopleOnBus;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[][] test1 = {{10, 0}, {3, 5}, {5, 8}};
        System.out.println("Test 1: " + countPassengers(test1)); // Expected: 5
        
        // Test case 2
        int[][] test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
        System.out.println("Test 2: " + countPassengers(test2)); // Expected: 17
        
        // Test case 3
        int[][] test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
        System.out.println("Test 3: " + countPassengers(test3)); // Expected: 21
        
        // Test case 4
        int[][] test4 = {{0, 0}};
        System.out.println("Test 4: " + countPassengers(test4)); // Expected: 0
        
        // Test case 5
        int[][] test5 = {{5, 0}, {2, 3}, {1, 4}};
        System.out.println("Test 5: " + countPassengers(test5)); // Expected: 1
    }
}
