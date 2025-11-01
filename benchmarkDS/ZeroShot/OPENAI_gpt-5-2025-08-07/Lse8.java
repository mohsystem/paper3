import java.util.Arrays;

public class Lse8 {
    // Returns the value at the index provided in argv[1], or null on invalid input.
    public static String getValueAtIndexFromArgs(String[] argv) {
        if (argv == null || argv.length < 2) {
            return null;
        }
        int idx;
        try {
            idx = Integer.parseInt(argv[1]);
        } catch (NumberFormatException e) {
            return null;
        }
        if (idx < 0 || idx >= argv.length) {
            return null;
        }
        return argv[idx];
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] tests = new String[][]{
            new String[]{"prog", "2", "a", "b", "c"},   // expects "a"
            new String[]{"prog", "0"},                  // expects "prog"
            new String[]{"prog", "10", "x"},            // expects null (out of range)
            new String[]{"prog", "-1", "x"},            // expects null (negative index)
            new String[]{"prog", "notanint", "x"}       // expects null (parse error)
        };

        for (String[] t : tests) {
            String result = getValueAtIndexFromArgs(t);
            System.out.println(String.valueOf(result));
        }
    }
}