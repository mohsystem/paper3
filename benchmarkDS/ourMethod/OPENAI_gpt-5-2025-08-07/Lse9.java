import java.nio.charset.StandardCharsets;

public class Lse9 {

    // Returns the argument at the given index from argv (which should include program name at index 0), or null if invalid.
    public static String argAtIndex(String[] argv, int index) {
        if (argv == null) {
            return null;
        }
        if (index < 0 || index >= argv.length) {
            return null;
        }
        return argv[index];
    }

    private static void runTests() {
        String[] sample = new String[] { "prog", "alpha", "beta", "gamma", "delta" };

        // 5 test cases
        int[] indexes = new int[] { 0, 1, 3, 4, 10 };
        for (int i = 0; i < indexes.length; i++) {
            String result = argAtIndex(sample, indexes[i]);
            if (result != null) {
                System.out.println(result);
            } else {
                System.out.println("ERROR");
            }
        }
    }

    public static void main(String[] args) {
        // If command-line args were provided, treat args[0] as the index (like ./argv_index 1 ...)
        if (args.length >= 1) {
            // Build a full argv including the program name at index 0 to match the specification
            String[] fullArgv = new String[args.length + 1];
            fullArgv[0] = "Lse9";
            System.arraycopy(args, 0, fullArgv, 1, args.length);

            // Parse index from args[0] (which is fullArgv[1])
            try {
                long idxLong = Long.parseLong(args[0]);
                if (idxLong < 0 || idxLong > Integer.MAX_VALUE) {
                    System.out.println("ERROR");
                    return;
                }
                int idx = (int) idxLong;
                String out = argAtIndex(fullArgv, idx);
                if (out != null) {
                    System.out.println(out);
                } else {
                    System.out.println("ERROR");
                }
            } catch (NumberFormatException e) {
                System.out.println("ERROR");
            }
        } else {
            // No args provided: run 5 test cases
            runTests();
        }
    }
}