import java.util.ArrayList;
import java.util.List;

public class Lse124 {
    public static List<String> run(String[] args) {
        List<String> out = new ArrayList<>();
        if (args == null || args.length != 1) {
            out.add("Error: expected exactly one argument.");
            return out;
        }
        String input = args[0];
        out.add(input);
        String trimmed = rtrim(input);
        out.add(trimmed);
        return out;
    }

    private static String rtrim(String s) {
        int end = s.length();
        while (end > 0 && Character.isWhitespace(s.charAt(end - 1))) {
            end--;
        }
        return s.substring(0, end);
    }

    public static void main(String[] args) {
        List<String[]> tests = new ArrayList<>();
        tests.add(new String[]{}); // no args
        tests.add(new String[]{"hello"});
        tests.add(new String[]{"hello   "});
        tests.add(new String[]{"world\t \t"});
        tests.add(new String[]{"a", "b"});

        for (String[] t : tests) {
            List<String> res = run(t);
            for (String line : res) {
                System.out.println(line);
            }
        }
    }
}