public class Task13 {

    /**
     * Determines if a given walk is valid.
     * A valid walk takes exactly 10 minutes and returns to the starting point.
     *
     * @param walk An array of characters representing directions ('n', 's', 'e', 'w').
     * @return true if the walk is valid, false otherwise.
     */
    public static boolean isValidWalk(char[] walk) {
        if (walk == null || walk.length != 10) {
            return false;
        }

        int x = 0; // East-West displacement
        int y = 0; // North-South displacement

        for (char direction : walk) {
            switch (direction) {
                case 'n': y++; break;
                case 's': y--; break;
                case 'e': x++; break;
                case 'w': x--; break;
                default:
                    // Per the prompt, input is always valid, so no action needed.
                    break;
            }
        }

        return x == 0 && y == 0;
    }

    public static void main(String[] args) {
        // Test Case 1: Valid walk
        char[] walk1 = {'n','s','n','s','n','s','n','s','n','s'};
        System.out.println("Test 1: " + isValidWalk(walk1)); // Expected: true

        // Test Case 2: Too long
        char[] walk2 = {'w','e','w','e','w','e','w','e','w','e','w','e'};
        System.out.println("Test 2: " + isValidWalk(walk2)); // Expected: false

        // Test Case 3: Too short
        char[] walk3 = {'w'};
        System.out.println("Test 3: " + isValidWalk(walk3)); // Expected: false

        // Test Case 4: Correct length, but does not return to start
        char[] walk4 = {'n','n','n','s','n','s','n','s','n','s'};
        System.out.println("Test 4: " + isValidWalk(walk4)); // Expected: false
        
        // Test Case 5: Valid walk, different order
        char[] walk5 = {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'};
        System.out.println("Test 5: " + isValidWalk(walk5)); // Expected: true
    }
}