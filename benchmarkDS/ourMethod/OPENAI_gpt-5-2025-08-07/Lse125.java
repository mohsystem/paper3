import java.nio.charset.StandardCharsets;

public class Lse125 {

    public static String trimTrailingWhitespace(String input) {
        if (input == null) {
            return "";
        }
        int end = input.length() - 1;
        while (end >= 0 && Character.isWhitespace(input.charAt(end))) {
            end--;
        }
        return input.substring(0, end + 1);
    }

    public static void main(String[] args) {
        // If a command-line argument is provided, trim trailing whitespace and print it.
        if (args.length > 0) {
            String trimmed = trimTrailingWhitespace(new String(args[0].getBytes(StandardCharsets.UTF_8), StandardCharsets.UTF_8));
            System.out.println(trimmed);
        }

        // Five test cases
        String[] tests = new String[] {
            "hello",
            "hello   ",
            "line\n",
            "tabs\t\t",
            "mix \t\n\r"
        };

        for (int i = 0; i < tests.length; i++) {
            String result = trimTrailingWhitespace(tests[i]);
            System.out.println("TEST " + (i + 1) + ": [" + result + "]");
        }
    }
}