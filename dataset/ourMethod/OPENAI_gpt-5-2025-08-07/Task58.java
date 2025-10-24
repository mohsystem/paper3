import java.util.Arrays;

public class Task58 {
    public static boolean XO(String s) {
        if (s == null) {
            return false; // fail closed on invalid input
        }
        int xCount = 0;
        int oCount = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == 'x' || c == 'X') {
                xCount++;
            } else if (c == 'o' || c == 'O') {
                oCount++;
            }
        }
        return xCount == oCount;
    }

    public static void main(String[] args) {
        String[] tests = new String[] { "ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo" };
        boolean[] expected = new boolean[] { true, false, true, true, false };
        for (int i = 0; i < tests.length; i++) {
            boolean res = XO(tests[i]);
            System.out.println("XO(\"" + tests[i] + "\") => " + res + " (expected " + expected[i] + ")");
        }
    }
}