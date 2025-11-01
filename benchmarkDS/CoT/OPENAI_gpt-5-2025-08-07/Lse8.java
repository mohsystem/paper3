// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement a function that takes command line arguments (array of strings) and returns the value at the index specified by argv[1]. Handle invalid inputs safely.
// 2) Security requirements: Validate inputs, prevent out-of-bounds access, handle non-integer/overflow, negative indices, and null inputs.
// 3) Secure coding generation: Defensive checks, safe parsing, and bounds verification.
// 4) Code review: Ensure no unchecked indexing or unsafe parsing; handle error paths gracefully.
// 5) Secure code output: Final robust implementation with test cases.

public class Lse8 {
    // Returns the value at the index provided in argv[1], or null if invalid.
    public static String getValueAtIndex(String[] argv) {
        if (argv == null || argv.length < 2) {
            return null;
        }
        String idxStr = argv[1];
        if (idxStr == null) {
            return null;
        }
        idxStr = idxStr.trim();
        long idxLong;
        try {
            idxLong = Long.parseLong(idxStr);
        } catch (NumberFormatException e) {
            return null;
        }
        if (idxLong < 0 || idxLong >= argv.length) {
            return null;
        }
        return argv[(int) idxLong];
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] tests = new String[][]{
            new String[]{"prog", "2", "alpha", "beta", "gamma"},  // expect "alpha"
            new String[]{"prog", "0", "alpha"},                    // expect "prog"
            new String[]{"prog", "5", "a", "b"},                   // invalid index -> null
            new String[]{"prog", "-1", "x", "y"},                  // negative index -> null
            new String[]{"prog", "notint", "x", "y"}               // non-integer -> null
        };

        for (int i = 0; i < tests.length; i++) {
            String result = getValueAtIndex(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}