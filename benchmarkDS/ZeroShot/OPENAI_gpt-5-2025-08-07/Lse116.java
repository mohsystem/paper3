import java.util.Locale;
import java.util.Arrays;
import java.util.List;
import java.io.IOException;

public class Lse116 {

    // Validate that the path contains only safe characters to avoid command injection
    static boolean isSafePath(String s) {
        if (s == null || s.isEmpty()) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') ||
                         (c >= '0' && c <= '9') ||
                         c == '/' || c == '.' || c == '_' || c == '-' || c == '+';
            if (!ok) return false;
        }
        return true;
    }

    // Print-like formatting function that returns the formatted string
    static String print_format(String fmt, Object... args) {
        return String.format(Locale.ROOT, fmt, args);
    }

    // "system" function to execute the command string
    static int systemCall(String cmd) {
        ProcessBuilder pb = new ProcessBuilder("/bin/sh", "-c", cmd);
        pb.inheritIO();
        try {
            Process p = pb.start();
            return p.waitFor();
        } catch (IOException | InterruptedException e) {
            return -1;
        }
    }

    // Function to execute ls with a sanitized path argument
    public static int lsCommand(String dir) {
        if (!isSafePath(dir)) {
            System.out.println("Rejected unsafe path: " + dir);
            return -1;
        }
        String cmd = print_format("/bin/ls -la -- %s", dir);
        return systemCall(cmd);
    }

    public static void main(String[] args) {
        List<String> tests = Arrays.asList(
            ".",
            "src",
            "/tmp",
            "does-not-exist",
            "bad;rm -rf /"
        );
        for (String t : tests) {
            System.out.println("Running ls on: " + t);
            int status = lsCommand(t);
            System.out.println("Exit status: " + status);
            System.out.println("----");
        }
    }
}