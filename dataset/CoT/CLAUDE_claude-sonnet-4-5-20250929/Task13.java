
public class Task13 {
    public static boolean isValidWalk(char[] walk) {
        // Check if walk is null to prevent NullPointerException
        if (walk == null) {
            return false;
        }
        
        // Check if the walk takes exactly 10 minutes
        if (walk.length != 10) {
            return false;
        }
        
        // Track position on grid
        int x = 0;
        int y = 0;
        
        // Process each direction
        for (int i = 0; i < walk.length; i++) {
            char direction = walk[i];
            
            // Validate that only allowed directions are present
            if (direction != 'n' && direction != 's' && direction != 'e' && direction != 'w') {
                return false;
            }
            
            // Update position based on direction
            switch (direction) {
                case 'n':
                    y++;
                    break;
                case 's':
                    y--;
                    break;
                case 'e':
                    x++;
                    break;
                case 'w':
                    x--;
                    break;
            }
        }
        
        // Check if we returned to starting point
        return x == 0 && y == 0;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid 10-minute walk returning to start
        char[] test1 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
        System.out.println("Test 1: " + isValidWalk(test1)); // Expected: true
        
        // Test case 2: Valid 10-minute walk with all directions
        char[] test2 = {'n', 's', 'e', 'w', 'n', 's', 'e', 'w', 'n', 's'};
        System.out.println("Test 2: " + isValidWalk(test2)); // Expected: true
        
        // Test case 3: Too short walk
        char[] test3 = {'n', 's', 'e', 'w'};
        System.out.println("Test 3: " + isValidWalk(test3)); // Expected: false
        
        // Test case 4: Correct length but doesn't return to start\n        char[] test4 = {'n', 'n', 'n', 's', 's', 's', 'e', 'w', 'n', 's'};\n        System.out.println("Test 4: " + isValidWalk(test4)); // Expected: false\n        \n        // Test case 5: Too long walk\n        char[] test5 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
        System.out.println("Test 5: " + isValidWalk(test5)); // Expected: false
    }
}
