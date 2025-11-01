import java.util.Arrays;

public class Lse124 {
    private static final int MAX_LEN = 1_048_576; // 1 MiB

    private static String rtrim(String s) {
        int end = s.length();
        while (end > 0 && Character.isWhitespace(s.charAt(end - 1))) {
            end--;
        }
        return s.substring(0, end);
    }

    public static int process(String[] argv) {
        if (argv == null || argv.length != 1) {
            System.err.println("Error: expected exactly one argument.");
            return 1;
        }
        String input = argv[0];
        if (input == null) {
            input = "";
        }
        if (input.length() > MAX_LEN) {
            System.err.println("Error: input too long.");
            return 1;
        }

        System.out.println(input);
        String trimmed = rtrim(input);
        System.out.println(trimmed);

        return 0;
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
                {"hello"},
                {"hello \t \n"},
                {},
                {"a", "b"},
                {"   "}
        };

        int caseNo = 1;
        for (String[] t : tests) {
            System.out.println("---- Test " + caseNo + " ----");
            int code = process(t);
            System.out.println("Exit code: " + code);
            caseNo++;
        }
    }
}