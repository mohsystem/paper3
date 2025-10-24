import java.util.Arrays;

public class Task13 {
    public static boolean isValidWalk(String[] walk) {
        if (walk == null || walk.length != 10) {
            return false;
        }
        int x = 0;
        int y = 0;
        for (int i = 0; i < walk.length; i++) {
            String s = walk[i];
            if (s == null || s.length() != 1) {
                return false;
            }
            char c = s.charAt(0);
            switch (c) {
                case 'n': y += 1; break;
                case 's': y -= 1; break;
                case 'e': x += 1; break;
                case 'w': x -= 1; break;
                default: return false;
            }
        }
        return x == 0 && y == 0;
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            new String[]{"n","s","n","s","n","s","n","s","n","s"}, // true
            new String[]{"w"},                                     // false (not 10)
            new String[]{"n","n","n","s","n","s","n","s","n","s"}, // false (not back)
            new String[]{"e","w","e","w","e","w","e","w","e","w"}, // true
            new String[]{"n","s","x","s","n","s","n","s","n","s"}  // false (invalid dir)
        };

        for (int i = 0; i < tests.length; i++) {
            boolean result = isValidWalk(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + result + " | Input=" + Arrays.toString(tests[i]));
        }
    }
}