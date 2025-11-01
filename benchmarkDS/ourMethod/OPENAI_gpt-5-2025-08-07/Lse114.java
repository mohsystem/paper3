import java.io.IOException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;

public class Lse114 {

    private static final Set<String> ALLOWED_FLAGS = new HashSet<>(
            Arrays.asList("", "-l", "-a", "-la", "-al", "-1", "-lh", "-lah", "-alh"));

    // "print" function: creates a formatted string from a format and variable args
    public static String printFormat(String fmt, Object... args) {
        return String.format(Locale.ROOT, fmt, args);
    }

    private static boolean isAllowedFlag(String flags) {
        String f = (flags == null) ? "" : flags;
        return ALLOWED_FLAGS.contains(f);
    }

    private static boolean isSafePath(String path) {
        if (path == null || path.isEmpty() || path.length() > 1024) return false;
        for (int i = 0; i < path.length(); i++) {
            char c = path.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '.' || c == '_' || c == '/' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    // "system" function: executes a command string via a shell
    public static int systemExec(String command) {
        if (command == null) return -1;
        ProcessBuilder pb = new ProcessBuilder("/bin/sh", "-c", command);
        pb.redirectErrorStream(true);
        pb.inheritIO();
        try {
            Process p = pb.start();
            return p.waitFor();
        } catch (IOException | InterruptedException e) {
            Thread.currentThread().interrupt();
            return -1;
        }
    }

    // Secure wrapper to build and execute an ls command
    public static int safeLs(String flags, String path) {
        String f = (flags == null) ? "" : flags;
        String p = (path == null || path.isEmpty()) ? "." : path;

        if (!isAllowedFlag(f)) {
            System.err.println("Invalid flags");
            return -1;
        }
        if (!isSafePath(p)) {
            System.err.println("Invalid path");
            return -1;
        }

        String cmd;
        if (!f.isEmpty()) {
            cmd = printFormat("ls %s %s", f, p);
        } else {
            cmd = printFormat("ls %s", p);
        }
        return systemExec(cmd);
    }

    public static void main(String[] args) {
        // 5 test cases
        int rc1 = safeLs(null, ".");
        System.out.println("Return code: " + rc1);

        int rc2 = safeLs("-l", ".");
        System.out.println("Return code: " + rc2);

        int rc3 = safeLs("-a", "/");
        System.out.println("Return code: " + rc3);

        int rc4 = safeLs("-lh", ".");
        System.out.println("Return code: " + rc4);

        int rc5 = safeLs("-rf", "."); // invalid, should fail validation
        System.out.println("Return code: " + rc5);
    }
}