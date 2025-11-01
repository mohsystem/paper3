import java.util.ArrayList;
import java.util.List;

public class Lse9 {

    public static String argAtIndex(int index, String[] argvIncludingProgramName) {
        if (argvIncludingProgramName == null) return null;
        if (index < 0 || index >= argvIncludingProgramName.length) return null;
        return argvIncludingProgramName[index];
    }

    private static Integer parseNonNegativeInt(String s) {
        if (s == null) return null;
        String t = s.trim();
        if (t.isEmpty()) return null;
        int start = 0;
        if (t.charAt(0) == '+') {
            start = 1;
        } else if (t.charAt(0) == '-') {
            return null;
        }
        for (int i = start; i < t.length(); i++) {
            char c = t.charAt(i);
            if (c < '0' || c > '9') return null;
        }
        try {
            long val = Long.parseLong(t);
            if (val < 0 || val > Integer.MAX_VALUE) return null;
            return (int) val;
        } catch (NumberFormatException e) {
            return null;
        }
    }

    public static void main(String[] args) {
        if (args != null && args.length > 0) {
            // Build an argv-like array including a placeholder program name at index 0
            String[] argv = new String[args.length + 1];
            argv[0] = "Lse9";
            System.arraycopy(args, 0, argv, 1, args.length);

            Integer idx = parseNonNegativeInt(argv[1]);
            if (idx == null) {
                System.err.println("Error: index must be a non-negative integer.");
                System.exit(1);
                return;
            }
            String result = argAtIndex(idx, argv);
            if (result == null) {
                System.err.println("Error: index out of range.");
                System.exit(1);
                return;
            }
            System.out.println(result);
            return;
        }

        // Test cases (run when no CLI args are provided)
        List<String[]> testArgvs = new ArrayList<>();
        testArgvs.add(new String[]{"prog", "1", "alpha", "beta", "gamma"});
        testArgvs.add(new String[]{"prog", "1", "alpha", "beta", "gamma"});
        testArgvs.add(new String[]{"prog", "1", "alpha", "beta", "gamma"});
        testArgvs.add(new String[]{"prog", "3", "foo", "bar", "baz"});
        testArgvs.add(new String[]{"prog", "0"});

        int[] indices = new int[]{0, 1, 2, 3, 0};

        for (int i = 0; i < 5; i++) {
            String out = argAtIndex(indices[i], testArgvs.get(i));
            if (out != null) {
                System.out.println(out);
            } else {
                System.out.println();
            }
        }
    }
}