import java.util.Locale;

public class Task44 {
    public static boolean isSafeAscii(String s) {
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch > 0x7F) return false;
            if (!(Character.isLetterOrDigit(ch) || ch == ' ' || ch == '_' || ch == '-')) return false;
        }
        return true;
    }

    public static String processData(int repeatCount, String text) {
        if (repeatCount < 1 || repeatCount > 20) {
            return "ERROR: repeatCount out of range [1,20]";
        }
        if (text == null) {
            return "ERROR: text is null";
        }
        String trimmed = text.trim();
        if (trimmed.length() < 1 || trimmed.length() > 100) {
            return "ERROR: text length must be 1..100 after trimming";
        }
        if (!isSafeAscii(trimmed)) {
            return "ERROR: text contains disallowed characters (allowed: A-Z, a-z, 0-9, space, underscore, hyphen)";
        }
        String normalized = trimmed.toUpperCase(Locale.ROOT);

        int checksum = 0;
        for (int i = 0; i < normalized.length(); i++) {
            checksum = (checksum + (normalized.charAt(i) & 0xFF)) % 1000;
        }

        StringBuilder repeated = new StringBuilder();
        for (int i = 0; i < repeatCount; i++) {
            if (i > 0) repeated.append('-');
            repeated.append(normalized);
        }

        String result = "{\"original\":\"" + trimmed + "\", " +
                "\"normalized\":\"" + normalized + "\", " +
                "\"repeatCount\":" + repeatCount + ", " +
                "\"repeated\":\"" + repeated + "\", " +
                "\"length\":" + normalized.length() + ", " +
                "\"checksum\":" + checksum + "}";

        return result;
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(processData(3, "hello world"));
        System.out.println(processData(1, "Safe_Input-123"));
        System.out.println(processData(5, "   trim  me   "));
        System.out.println(processData(0, "abc")); // invalid repeatCount
        System.out.println(processData(2, "bad!char")); // invalid character '!'

        // Optional user input via command-line args: first arg is repeatCount, the rest form the text
        if (args != null && args.length >= 2) {
            try {
                int rc = Integer.parseInt(args[0]);
                StringBuilder sb = new StringBuilder();
                for (int i = 1; i < args.length; i++) {
                    if (i > 1) sb.append(' ');
                    sb.append(args[i]);
                }
                System.out.println(processData(rc, sb.toString()));
            } catch (NumberFormatException e) {
                System.out.println("ERROR: first argument must be an integer repeatCount");
            }
        }
    }
}