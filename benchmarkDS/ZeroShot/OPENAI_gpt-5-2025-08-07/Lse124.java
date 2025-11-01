import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Lse124 {
    public static final class Result {
        public final List<String> lines;
        public final int exitCode;
        public Result(List<String> lines, int exitCode) {
            this.lines = lines;
            this.exitCode = exitCode;
        }
    }

    private static String rtrim(String s) {
        if (s == null) return "";
        int end = s.length();
        while (end > 0 && Character.isWhitespace(s.charAt(end - 1))) {
            end--;
        }
        return s.substring(0, end);
    }

    // Function that accepts input as parameters and returns the output (and exit code)
    public static Result processArgs(String[] args) {
        List<String> out = new ArrayList<>();
        if (args == null || args.length != 1) {
            out.add("Error: expected exactly one argument.");
            return new Result(out, 1);
        }
        String input = args[0] == null ? "" : args[0];
        out.add(input);
        String trimmed = rtrim(input);
        out.add(trimmed);
        return new Result(out, 0);
    }

    public static void main(String[] args) {
        if (args != null && args.length > 0) {
            Result res = processArgs(args);
            for (String line : res.lines) {
                System.out.println(line);
            }
            System.exit(res.exitCode);
        } else {
            // 5 test cases
            String[][] tests = new String[][]{
                new String[]{},                      // 1) zero args -> error
                new String[]{"hello"},               // 2) single arg, no trailing ws
                new String[]{"abc   "},              // 3) trailing spaces
                new String[]{"foo \t"},              // 4) trailing tab
                new String[]{"one", "two"}           // 5) multiple args -> error
            };
            for (int i = 0; i < tests.length; i++) {
                System.out.println("=== Test " + (i + 1) + " ===");
                Result res = processArgs(tests[i]);
                for (String line : res.lines) {
                    System.out.println(line);
                }
                System.out.println("Exit code: " + res.exitCode);
            }
        }
    }
}