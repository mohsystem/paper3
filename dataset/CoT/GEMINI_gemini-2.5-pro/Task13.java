import java.util.Arrays;

public class Task13 {

    public static boolean isValidWalk(char[] walk) {
        // 1. The walk must take exactly 10 minutes.
        if (walk.length != 10) {
            return false;
        }

        // 2. The walk must return you to your starting point.
        int x = 0; // Represents East-West axis
        int y = 0; // Represents North-South axis

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
                // No default case needed as input is guaranteed to be valid.
            }
        }

        return x == 0 && y == 0;
    }

    public static void main(String[] args) {
        // Test Cases
        char[][] testCases = {
            {'n','s','n','s','n','s','n','s','n','s'}, // true
            {'w','e','w','e','w','e','w','e','w','e'}, // true
            {'w'},                                    // false (too short)
            {'n','n','n','s','n','s','n','s','n','s'}, // false (doesn't return to start)
            {'e','e','e','e','w','w','w','w','w','w'}  // false (doesn't return to start)
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": " + Arrays.toString(testCases[i]));
            System.out.println("Result: " + isValidWalk(testCases[i]));
            System.out.println();
        }
    }
}