import java.util.regex.Pattern;

public class Lse8 {

    // Returns the value at the index specified by argv[1] within argv.
    // Throws IllegalArgumentException on invalid input.
    public static String valueAtIndex(String[] argv) {
        if (argv == null || argv.length < 2) {
            throw new IllegalArgumentException("insufficient arguments");
        }

        String idxStr = argv[1];
        // Only allow non-empty digits, cap length to avoid overflow
        if (idxStr == null || !Pattern.matches("\\d{1,10}", idxStr)) {
            throw new IllegalArgumentException("invalid index");
        }

        long idx;
        try {
            idx = Long.parseLong(idxStr);
        } catch (NumberFormatException ex) {
            throw new IllegalArgumentException("invalid index");
        }

        if (idx > Integer.MAX_VALUE) {
            throw new IllegalArgumentException("invalid index");
        }

        int i = (int) idx;
        if (i < 0 || i >= argv.length) {
            throw new IllegalArgumentException("index out of range");
        }

        return argv[i];
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            new String[]{"prog", "0"},
            new String[]{"app", "2", "x", "y", "z"},
            new String[]{"cmd", "4", "A", "B", "C", "D", "E"},
            new String[]{"tool", "-1", "val"},
            new String[]{"t", "10", "a", "b"}
        };

        for (int t = 0; t < tests.length; t++) {
            try {
                String result = valueAtIndex(tests[t]);
                System.out.println("Test " + (t + 1) + " -> " + result);
            } catch (IllegalArgumentException ex) {
                System.out.println("Test " + (t + 1) + " -> error: " + ex.getMessage());
            }
        }
    }
}