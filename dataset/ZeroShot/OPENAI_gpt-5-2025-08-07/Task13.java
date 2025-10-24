public class Task13 {
    public static boolean isValidWalk(String[] walk) {
        if (walk == null || walk.length != 10) return false;
        int x = 0, y = 0;
        for (String dir : walk) {
            if (dir == null || dir.length() != 1) return false;
            char c = dir.charAt(0);
            switch (c) {
                case 'n': y++; break;
                case 's': y--; break;
                case 'e': x++; break;
                case 'w': x--; break;
                default: return false;
            }
        }
        return x == 0 && y == 0;
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            {"n","s","n","s","n","s","n","s","n","s"},
            {"w","e","w","e","w","e","w","e","w","e","w","e"},
            {"w","w","w","w","w","w","w","w","w","w"},
            {"n","s","n","s","n","s","n","s","n","x"},
            {"n","n","e","e","s","s","w","w","n","s"}
        };
        boolean[] expected = new boolean[]{true, false, false, false, true};
        for (int i = 0; i < tests.length; i++) {
            boolean result = isValidWalk(tests[i]);
            System.out.println("Test " + (i+1) + ": " + result + " (expected " + expected[i] + ")");
        }
    }
}