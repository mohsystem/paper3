public class Lse123 {
    public static String process(String[] args) {
        // First if: ensure exactly one argument
        if (args == null || args.length != 1) {
            return null;
        }
        // Check the argument is a string
        if (args[0] == null) {
            return null;
        }

        char[] arr = args[0].toCharArray();

        // For loop trims trailing whitespace (spaces and tabs)
        int end = arr.length - 1;
        for (; end >= 0 && (arr[end] == ' ' || arr[end] == '\t'); end--) {
            // trimming
        }
        String trimmed = new String(arr, 0, Math.max(0, end + 1));

        // Second for loop trims the newlines (removes all '\n' and '\r')
        StringBuilder sb = new StringBuilder();
        char[] tarr = trimmed.toCharArray();
        for (int i = 0; i < tarr.length; i++) {
            char c = tarr[i];
            if (c != '\n' && c != '\r') {
                sb.append(c);
            }
        }
        // In Java, strings are inherently null-terminated by the runtime (not manually applicable)
        return sb.toString();
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            new String[]{"Hello World   "},
            new String[]{"Line with newline\n\n"},
            new String[]{" Mix \t \nLine\r\nBreaks\t\t"},
            new String[]{},                // invalid: 0 args
            new String[]{"A", "B"}         // invalid: >1 args
        };

        for (int i = 0; i < tests.length; i++) {
            String result = process(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + (result == null ? "null" : "\"" + result + "\""));
        }
    }
}