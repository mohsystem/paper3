public class Task13 {

    /**
     * Checks if a given walk is valid.
     * A valid walk takes exactly 10 minutes (10 steps) and returns to the starting point.
     *
     * @param walk an array of characters representing directions ('n', 's', 'e', 'w').
     * @return true if the walk is valid, false otherwise.
     */
    public static boolean isValidWalk(char[] walk) {
        // The walk must be exactly 10 minutes long.
        if (walk == null || walk.length != 10) {
            return false;
        }

        int x = 0; // Represents horizontal displacement (East/West)
        int y = 0; // Represents vertical displacement (North/South)

        for (char direction : walk) {
            switch (direction) {
                case 'n': y++; break;
                case 's': y--; break;
                case 'e': x++; break;
                case 'w': x--; break;
                // According to the prompt, we don't need to handle other cases.
            }
        }

        // The walk must return to the starting point (0,0).
        return x == 0 && y == 0;
    }

    public static void main(String[] args) {
        // Test Case 1: Valid walk
        char[] walk1 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
        System.out.println("Test Case 1: " + (isValidWalk(walk1) ? "true" : "false")); // Expected: true

        // Test Case 2: Too long walk
        char[] walk2 = {'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e'};
        System.out.println("Test Case 2: " + (isValidWalk(walk2) ? "true" : "false")); // Expected: false

        // Test Case 3: Too short walk
        char[] walk3 = {'w'};
        System.out.println("Test Case 3: " + (isValidWalk(walk3) ? "true" : "false")); // Expected: false

        // Test Case 4: Correct length, but does not return to start
        char[] walk4 = {'n', 'n', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
        System.out.println("Test Case 4: " + (isValidWalk(walk4) ? "true" : "false")); // Expected: false

        // Test Case 5: Valid walk with mixed directions
        char[] walk5 = {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'};
        System.out.println("Test Case 5: " + (isValidWalk(walk5) ? "true" : "false")); // Expected: true
    }
}