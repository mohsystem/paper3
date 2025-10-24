import java.util.Arrays;

public class Task44 {
    public static String processInput(String input) {
        if (input == null) {
            return "Invalid input";
        }
        if (input.length() > 256) {
            return "Invalid input";
        }
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (!isAllowed(c)) {
                return "Invalid input";
            }
        }
        StringBuilder sb = new StringBuilder(input.length());
        boolean lastWasSpace = true; // treat start as space to trim leading
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (c == ' ' || c == '\t') {
                if (!lastWasSpace) {
                    sb.append(' ');
                    lastWasSpace = true;
                }
            } else {
                sb.append(c);
                lastWasSpace = false;
            }
        }
        int len = sb.length();
        if (len > 0 && sb.charAt(len - 1) == ' ') {
            sb.setLength(len - 1);
        }
        String sanitized = sb.toString();
        if (sanitized.isEmpty()) {
            return "Invalid input";
        }
        int digitCount = 0;
        for (int i = 0; i < sanitized.length(); i++) {
            char c = sanitized.charAt(i);
            if (c >= '0' && c <= '9') digitCount++;
        }
        String reversed = new StringBuilder(sanitized).reverse().toString();
        return "sanitized=" + sanitized + "; length=" + sanitized.length() + "; digits=" + digitCount + "; reversed=" + reversed;
    }

    private static boolean isAllowed(char c) {
        if (c >= 'a' && c <= 'z') return true;
        if (c >= 'A' && c <= 'Z') return true;
        if (c >= '0' && c <= '9') return true;
        if (c == ' ' || c == '\t') return true;
        switch (c) {
            case '_':
            case '-':
            case '.':
            case ',':
            case ':':
            case '@':
                return true;
            default:
                return false;
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hello World",
            "  Multiple    spaces   123  ",
            "Invalid!Chars#",
            "A_Very-Long.String,With:Allowed@Chars",
            ""
        };
        for (String t : tests) {
            String result = processInput(t);
            System.out.println("Input: " + Arrays.toString(new String[]{t}) + " -> " + result);
        }
    }
}