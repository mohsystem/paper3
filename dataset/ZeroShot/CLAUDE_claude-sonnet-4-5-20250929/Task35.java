
public class Task35 {
    public static int countPassengers(int[][] busStops) {
        int peopleOnBus = 0;
        
        for (int i = 0; i < busStops.length; i++) {
            peopleOnBus += busStops[i][0]; // people getting on
            peopleOnBus -= busStops[i][1]; // people getting off
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
        int[][] test5 = {{5, 0}, {2, 3}, {10, 5}, {4, 8}};
        System.out.println("Test 5: " + countPassengers(test5)); // Expected: 5
    }
}
