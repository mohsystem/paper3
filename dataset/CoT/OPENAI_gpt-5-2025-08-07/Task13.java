// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement a function that, given an array of one-letter direction strings ('n','s','e','w'),
//    returns true if the walk takes exactly 10 steps and returns to the origin.
// 2) Security requirements: Validate inputs (null checks, length checks, allowed characters). Avoid side effects.
// 3) Secure coding generation: Use immutable operations where possible and guard against invalid inputs.
// 4) Code review: Ensure all branches are handled; handle unexpected characters safely by returning false.
// 5) Secure code output: Final code uses clear, safe constructs and includes tests without external input.
public class Task13 {

    public static boolean isValidWalk(String[] walk) {
        if (walk == null) return false;
        if (walk.length != 10) return false;

        int x = 0, y = 0;
        for (String step : walk) {
            if (step == null || step.length() != 1) return false;
            char c = step.charAt(0);
            switch (c) {
                case 'n': y++; break;
                case 's': y--; break;
                case 'e': x++; break;
                case 'w': x--; break;
                default: return false; // invalid direction
            }
        }
        return x == 0 && y == 0;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] tests = new String[][]{
            {"n","s","n","s","n","s","n","s","n","s"}, // true
            {"w","e","w","e","w"},                     // false (length != 10)
            {"n","n","n","s","n","s","n","s","n","s"}, // false (doesn't return to start)
            {"e","w","e","w","e","w","e","w","e","w"}, // true
            {"n","n","n","n","n","s","s","s","s","e"}  // false (net east)
        };

        for (int i = 0; i < tests.length; i++) {
            boolean result = isValidWalk(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}