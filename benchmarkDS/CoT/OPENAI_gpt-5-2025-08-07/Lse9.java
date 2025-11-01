import java.util.Arrays;

public class Lse9 {
    public static String getArgAtIndex(String[] argv, int index) {
        if (argv == null) return null;
        if (index < 0 || index >= argv.length) return null;
        return argv[index];
    }

    private static void runTests() {
        String[] testArgv = new String[] {"prog", "1", "alpha", "beta", "gamma"};
        int[] testIndices = new int[] {1, 2, 4, 0, 10};
        for (int idx : testIndices) {
            String res = getArgAtIndex(testArgv, idx);
            if (res != null) {
                System.out.println(res);
            } else {
                System.out.println("ERROR");
            }
        }
    }

    public static void main(String[] args) {
        if (args.length == 0) {
            runTests();
            return;
        }
        int idx;
        try {
            idx = Integer.parseInt(args[0].trim());
            if (idx < 0) {
                System.err.println("Error: index must be non-negative");
                return;
            }
        } catch (NumberFormatException e) {
            System.err.println("Error: invalid index");
            return;
        }
        String[] argvFull = new String[args.length + 1];
        argvFull[0] = "Lse9";
        System.arraycopy(args, 0, argvFull, 1, args.length);

        String result = getArgAtIndex(argvFull, idx);
        if (result == null) {
            System.err.println("Error: index out of range");
        } else {
            System.out.println(result);
        }
    }
}