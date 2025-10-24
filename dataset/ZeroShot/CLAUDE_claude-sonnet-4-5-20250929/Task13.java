
public class Task13 {
    public static boolean isValidWalk(char[] walk) {
        if (walk.length != 10) {
            return false;
        }
        
        int x = 0, y = 0;
        
        for (char direction : walk) {
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
        
        return x == 0 && y == 0;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid walk
        char[] test1 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
        System.out.println("Test 1: " + isValidWalk(test1)); // Expected: true
        
        // Test case 2: Valid walk with all directions
        char[] test2 = {'n', 's', 'e', 'w', 'n', 's', 'e', 'w', 'n', 's'};
        System.out.println("Test 2: " + isValidWalk(test2)); // Expected: true
        
        // Test case 3: Too short
        char[] test3 = {'n', 's', 'n', 's', 'n'};
        System.out.println("Test 3: " + isValidWalk(test3)); // Expected: false
        
        // Test case 4: Correct length but doesn't return to start\n        char[] test4 = {'n', 'n', 'n', 'n', 'n', 'n', 'n', 'n', 'n', 'n'};\n        System.out.println("Test 4: " + isValidWalk(test4)); // Expected: false\n        \n        // Test case 5: Too long\n        char[] test5 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
        System.out.println("Test 5: " + isValidWalk(test5)); // Expected: false
    }
}
