import java.util.*;

public class Lse9 {
    public static String getArgAt(String[] argv, int index) {
        if (index >= 0 && index < argv.length) return argv[index];
        return null;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[][] testArrays = {
            {"prog", "alpha", "beta", "gamma"},
            {"prog"},
            {"app", "x"},
            {"p", "10", "20", "30"},
            {"name", "first", "second"}
        };
        int[] testIndices = {1, 0, 5, 3, 2};
        for (int i = 0; i < 5; i++) {
            String res = getArgAt(testArrays[i], testIndices[i]);
            System.out.println(res != null ? res : "Index out of range");
        }

        // Handle actual command line
        if (args.length >= 1) {
            try {
                int idx = Integer.parseInt(args[0]);
                String[] full = new String[args.length + 1];
                full[0] = "Lse9";
                System.arraycopy(args, 0, full, 1, args.length);
                String res = getArgAt(full, idx);
                System.out.println(res != null ? res : "Index out of range");
            } catch (NumberFormatException e) {
                System.out.println("Invalid index");
            }
        }
    }
}