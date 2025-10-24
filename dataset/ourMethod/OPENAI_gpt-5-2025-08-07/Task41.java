import java.io.Console;
import java.util.Arrays;

public final class Task41 {

    private static final int MAX_INPUT_LEN = 1024;

    public static String processInput(final String input) {
        if (input == null) {
            return "";
        }
        final String truncated = (input.length() > MAX_INPUT_LEN) ? input.substring(0, MAX_INPUT_LEN) : input;

        StringBuilder sb = new StringBuilder(truncated.length());
        boolean inSpace = true; // treat leading spaces as to-be-skipped
        for (int i = 0; i < truncated.length(); i++) {
            char ch = truncated.charAt(i);
            if (Character.isWhitespace(ch)) {
                if (!inSpace && sb.length() > 0) {
                    sb.append(' ');
                }
                inSpace = true;
            } else {
                sb.append(Character.toUpperCase(ch));
                inSpace = false;
            }
        }
        // Remove trailing space if any
        int len = sb.length();
        if (len > 0 && sb.charAt(len - 1) == ' ') {
            sb.setLength(len - 1);
        }
        return sb.toString();
    }

    private static void runTests() {
        String[] tests = new String[5];
        tests[0] = "";
        tests[1] = "Hello, World!";
        tests[2] = "   multiple   spaces   here    ";
        tests[3] = "1234567890";
        char[] longArr = new char[1100];
        Arrays.fill(longArr, 'a');
        tests[4] = new String(longArr);

        for (int i = 0; i < tests.length; i++) {
            String out = processInput(tests[i]);
            System.out.println("Test " + (i + 1) + " Input: [" + tests[i] + "]");
            System.out.println("Test " + (i + 1) + " Output: [" + out + "]");
        }
    }

    public static void main(String[] args) {
        runTests();

        String userInput = null;
        if (args != null && args.length > 0) {
            userInput = args[0];
        } else {
            Console console = System.console();
            if (console != null) {
                String line = console.readLine("Enter a string to process (max %d chars): ", MAX_INPUT_LEN);
                if (line != null) {
                    userInput = line;
                }
            }
        }

        if (userInput != null) {
            String result = processInput(userInput);
            System.out.println("User Input Output: [" + result + "]");
        }
    }
}