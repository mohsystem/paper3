
import java.util.Arrays;
import java.util.List;

public class Task13 {
    public static boolean isValidWalk(char[] walk) {
        // Validate input: check for null
        if (walk == null) {
            return false;
        }
        
        // Check if walk is exactly 10 minutes
        if (walk.length != 10) {
            return false;
        }
        
        // Track position
        int x = 0;
        int y = 0;
        
        // Process each direction
        for (char direction : walk) {
            // Validate each direction character
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
                default:
                    // Invalid direction
                    return false;
            }
        }
        
        // Check if we're back at starting point\n        return x == 0 && y == 0;\n    }\n    \n    public static void main(String[] args) {\n        // Test case 1: Valid 10-minute walk returning to start\n        char[] test1 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};\n        System.out.println("Test 1: " + isValidWalk(test1)); // true\n        \n        // Test case 2: Valid 10-minute walk returning to start\n        char[] test2 = {'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e'};\n        System.out.println("Test 2: " + isValidWalk(test2)); // true\n        \n        // Test case 3: 10 minutes but doesn't return to start
        char[] test3 = {'n', 'n', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
        System.out.println("Test 3: " + isValidWalk(test3)); // false
        
        // Test case 4: Returns to start but not 10 minutes
        char[] test4 = {'n', 's', 'w', 'e'};
        System.out.println("Test 4: " + isValidWalk(test4)); // false
        
        // Test case 5: Valid complex path
        char[] test5 = {'w', 'w', 'w', 'e', 'n', 'n', 's', 's', 'e', 'e'};
        System.out.println("Test 5: " + isValidWalk(test5)); // true
    }
}
