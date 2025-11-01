import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse123 {
    private static final int MAX_LEN = 1 << 20; // 1 MiB max input length

    // Function: checks exactly one argument, validates it's a string, trims trailing whitespace then newlines, and null-terminates (simulated) before returning the trimmed string.
    public static String processArgs(String[] args) {
        if (args == null || args.length != 1) {
            throw new IllegalArgumentException("invalid args");
        }
        String s = args[0];
        if (s == null) {
            throw new IllegalArgumentException("invalid arg");
        }
        if (s.length() > MAX_LEN) {
            throw new IllegalArgumentException("too long");
        }
        // Work on a mutable char array
        char[] buf = s.toCharArray();
        int end = buf.length;

        // First loop: trim trailing whitespace excluding CR/LF (space, tab, vertical tab, form feed)
        int i = end - 1;
        while (i >= 0) {
            char c = buf[i];
            if (c == ' ' || c == '\t' || c == '\f' || c == '\u000B') {
                i--;
            } else {
                break;
            }
        }
        int end1 = i + 1;

        // Second loop: trim trailing newlines (LF, CR)
        int j = end1 - 1;
        while (j >= 0) {
            char c = buf[j];
            if (c == '\n' || c == '\r') {
                j--;
            } else {
                break;
            }
        }
        int end2 = j + 1;

        // Last line: "null terminate" by creating an array with explicit '\0'
        char[] outArr = new char[end2 + 1];
        if (end2 > 0) {
            System.arraycopy(buf, 0, outArr, 0, end2);
        }
        outArr[end2] = '\0'; // explicit terminator in the buffer (not included in returned String)
        return new String(outArr, 0, end2);
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
                { "hello   " },           // trailing spaces
                { "world\t\t\n" },        // tabs then newline
                { "line with spaces \r\n\n" }, // spaces then CRLF+LF
                { "no-trim" },            // no trailing whitespace/newlines
                { "invalid", "extra" }    // invalid: more than one argument
        };

        for (int t = 0; t < tests.length; t++) {
            try {
                String result = processArgs(tests[t]);
                // Print as UTF-8 safe; show length to verify trimming
                System.out.println("Test " + (t + 1) + " -> [" + result + "] len=" + result.length());
            } catch (IllegalArgumentException ex) {
                System.out.println("Test " + (t + 1) + " -> error");
            }
        }
    }
}