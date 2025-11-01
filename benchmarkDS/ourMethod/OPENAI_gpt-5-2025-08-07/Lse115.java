import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.Locale;
import java.util.concurrent.TimeUnit;

public class Lse115 {

    // Create a formatted string from a format and arguments, then validate for safe path usage
    public static String buildString(String format, Object... args) {
        if (format == null) {
            return "";
        }
        String s;
        try {
            s = String.format(Locale.ROOT, format, args);
        } catch (Exception e) {
            return "";
        }
        if (!isSafePath(s)) {
            return "";
        }
        return s;
    }

    // Execute the 'ls' command securely without invoking a shell. Returns the command output.
    public static String executeLs(String input) {
        String path = (input == null || input.isEmpty()) ? "." : input;
        if (!isSafePath(path)) {
            return "error: invalid input\n";
        }

        ProcessBuilder pb = new ProcessBuilder("ls", "--", path);
        pb.redirectErrorStream(true);
        StringBuilder out = new StringBuilder();
        Process p = null;
        try {
            p = pb.start();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
                String line;
                while ((line = br.readLine()) != null) {
                    out.append(line).append('\n');
                }
            }
            // Wait up to 15 seconds to avoid hanging
            if (!p.waitFor(15, TimeUnit.SECONDS)) {
                p.destroyForcibly();
                return "error: timeout\n";
            }
        } catch (Exception e) {
            if (p != null) {
                p.destroyForcibly();
            }
            return "error: execution failed\n";
        }
        return out.toString();
    }

    // Validate that the path is only composed of safe characters and does not include traversal
    private static boolean isSafePath(String s) {
        if (s == null) return false;
        if (s.length() > 255) return false;
        if (s.contains("\u0000")) return false;
        // Disallow path traversal
        if (s.contains("..")) return false;
        // Only allow alphanumerics and the characters . _ - /
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '.' || c == '_' || c == '-' || c == '/';
            if (!ok) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[5];
        tests[0] = buildString("%s", ".");              // current directory
        tests[1] = buildString("%s", "/");              // root
        tests[2] = buildString("nonexistent_%s", "dir");// a likely nonexistent dir
        tests[3] = buildString("%s/%s", "tmp", "test"); // nested path
        tests[4] = buildString("%s%s", ".", "");        // another current directory

        for (int i = 0; i < tests.length; i++) {
            String input = tests[i];
            if (input == null) input = "";
            System.out.println("=== Test " + (i + 1) + " ===");
            String output = executeLs(input);
            System.out.print(output);
        }
    }
}