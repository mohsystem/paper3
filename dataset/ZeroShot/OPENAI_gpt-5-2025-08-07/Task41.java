import java.util.*;

public class Task41 {
    // Processes the input string by removing control characters (except whitespace),
    // collapsing consecutive whitespace to a single space, and trimming.
    public static String processInput(String input) {
        if (input == null) return "";
        StringBuilder sb = new StringBuilder();
        boolean inSpace = false;

        input.codePoints().forEach(cp -> {
            boolean isWhitespace = Character.isWhitespace(cp);
            boolean isControl = Character.isISOControl(cp) && !isWhitespace;

            if (isControl) {
                // Skip non-whitespace control characters
                return;
            }

            if (isWhitespace) {
                if (sb.length() > 0 && !inSpace) {
                    sb.append(' ');
                    inSpace = true;
                }
            } else {
                sb.appendCodePoint(cp);
                inSpace = false;
            }
        });

        int len = sb.length();
        while (len > 0 && sb.charAt(len - 1) == ' ') {
            len--;
        }
        return sb.substring(0, len);
    }

    public static void main(String[] args) {
        // Five test cases
        String[] tests = new String[] {
            "Hello,  World!",                          // double spaces
            "   Leading and trailing   ",              // trim and collapse spaces
            "Line1\t\tLine2",                          // tabs collapsed to single space
            "Control\u0007Char and \u0009 tabs",       // remove bell, keep whitespace collapsed
            "aaaaa     bbbbb     ccccc    "            // collapse spaces and trim
        };

        for (int i = 0; i < tests.length; i++) {
            String processed = processInput(tests[i]);
            System.out.println("Test " + (i+1) + ": [" + processed + "]");
        }

        // If user provides an argument, treat it as user input to process
        if (args != null && args.length > 0) {
            String userInput = String.join(" ", args);
            String processed = processInput(userInput);
            System.out.println("User Input: [" + processed + "]");
        }
    }
}