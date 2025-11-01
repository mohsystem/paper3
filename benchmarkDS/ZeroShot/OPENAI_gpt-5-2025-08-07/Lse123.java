import java.util.Arrays;

public class Lse123 {
    private static boolean isWhitespaceButNotNewline(char c) {
        return c == ' ' || c == '\t' || c == '\f' || c == '\u000B';
    }

    private static boolean isNewline(char c) {
        return c == '\n' || c == '\r';
    }

    public static String sanitizeArgument(Object arg) {
        if (arg == null) {
            throw new IllegalArgumentException("Argument cannot be null.");
        }
        if (!(arg instanceof String)) {
            throw new IllegalArgumentException("Argument must be a string.");
        }
        String s = (String) arg;
        char[] chars = s.toCharArray();
        int end = chars.length - 1;

        for (; end >= 0 && isWhitespaceButNotNewline(chars[end]); end--) {
            // trimming trailing whitespace except newlines
        }
        for (; end >= 0 && isNewline(chars[end]); end--) {
            // trimming trailing newlines
        }

        int newLen = (end >= 0) ? (end + 1) : 0;
        return new String(chars, 0, newLen);
    }

    public static void main(String[] args) {
        if (args.length == 1) {
            try {
                String result = sanitizeArgument(args[0]);
                System.out.println(result);
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
                System.exit(1);
            }
        } else {
            // 5 test cases
            Object[] tests = new Object[] {
                "Hello World   ",
                "Tabs and spaces\t \t   ",
                "Ends with newlines\n\n\r",
                "Mix \t\v\f\n\r",
                "",
            };
            for (Object t : tests) {
                try {
                    String res = sanitizeArgument(t);
                    System.out.println("[" + res + "]");
                } catch (Exception e) {
                    System.out.println("Error: " + e.getMessage());
                }
            }
        }
    }
}