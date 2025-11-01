import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

public class Lse116 {

    // Build a command string using a format string and variable arguments (print-like).
    public static String printFormat(String format, Object... args) {
        if (format == null) {
            return "";
        }
        String cmd = args == null || args.length == 0
                ? format
                : String.format(Locale.ROOT, format, args);
        cmd = cmd.trim();
        // Enforce length limit
        if (cmd.length() == 0 || cmd.length() > 256) {
            return "";
        }
        return cmd;
    }

    // Validate command is a safe "ls" invocation with at most one simple argument
    private static boolean isSafeLsCommand(String cmd) {
        if (cmd == null) return false;
        String s = cmd.trim();
        if (s.length() < 2 || s.length() > 256) return false;
        if (!s.startsWith("ls")) return false;
        if (s.length() == 2) return true; // exactly "ls"
        if (s.charAt(2) != ' ') return false; // require a single space after "ls"
        String arg = s.substring(3);
        if (arg.isEmpty()) return false;
        if (arg.length() > 200) return false;
        if (arg.contains("..")) return false; // block traversal
        if (arg.indexOf(' ') >= 0) return false; // disallow spaces in arg for safety
        for (int i = 0; i < arg.length(); i++) {
            char c = arg.charAt(i);
            boolean ok = Character.isLetterOrDigit(c) || c == '/' || c == '.' || c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    // A "system"-like function that executes the provided command string safely.
    // Returns the exit code (0 usually means success).
    public static int system(String command) {
        if (!isSafeLsCommand(command)) {
            System.out.println("Rejected command");
            return -1;
        }
        String[] tokens;
        if (command.length() == 2) {
            tokens = new String[] {"ls"};
        } else {
            String arg = command.substring(3);
            tokens = new String[] {"ls", arg};
        }
        ProcessBuilder pb = new ProcessBuilder(tokens);
        pb.redirectErrorStream(true);
        Process p = null;
        int exitCode = -1;
        try {
            p = pb.start();
            try (BufferedReader br = new BufferedReader(
                    new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
                String line;
                while ((line = br.readLine()) != null) {
                    System.out.println(line);
                }
            }
            exitCode = p.waitFor();
        } catch (Exception e) {
            // Return generic failure, avoid leaking details
            exitCode = -1;
        } finally {
            if (p != null) p.destroy();
        }
        return exitCode;
    }

    // Convenience function that formats then runs ls, returning the exit code.
    public static int runLsFormatted(String format, Object... args) {
        String cmd = printFormat(format, args);
        return system(cmd);
    }

    public static void main(String[] args) {
        // 5 test cases
        int r1 = runLsFormatted("ls");
        System.out.println("Exit: " + r1);

        int r2 = runLsFormatted("ls %s", ".");
        System.out.println("Exit: " + r2);

        int r3 = runLsFormatted("ls %s", "/");
        System.out.println("Exit: " + r3);

        int r4 = runLsFormatted("ls %s", "./");
        System.out.println("Exit: " + r4);

        int r5 = runLsFormatted("ls %s", "nonexistent_path_should_fail");
        System.out.println("Exit: " + r5);
    }
}