import java.io.IOException;
import java.util.Locale;

public class Lse114 {
    // Emulate a "system" function that takes a string and executes it via a shell
    public static int system(String cmd) throws IOException, InterruptedException {
        ProcessBuilder pb = new ProcessBuilder("/bin/sh", "-c", cmd);
        pb.inheritIO();
        Process p = pb.start();
        return p.waitFor();
    }

    // "Print" function that formats a string with placeholders and variable args
    public static String printFormat(String fmt, Object... args) {
        return String.format(Locale.ROOT, fmt, args);
    }

    // Safely shell-quote a single argument
    public static String shellQuote(String s) {
        if (s == null || s.isEmpty()) {
            return "''";
        }
        StringBuilder sb = new StringBuilder();
        sb.append('\'');
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '\'') {
                sb.append("'\\''");
            } else {
                sb.append(c);
            }
        }
        sb.append('\'');
        return sb.toString();
    }

    // Build the ls command string using the print function, then pass to system
    public static int runLs(String path) throws IOException, InterruptedException {
        final String base = "/bin/ls -la";
        final String cmd;
        if (path != null && !path.isEmpty()) {
            String quoted = shellQuote(path);
            cmd = printFormat("%s %s", base, quoted);
        } else {
            cmd = base;
        }
        return system(cmd);
    }

    // 5 test cases
    public static void main(String[] args) throws Exception {
        String[] tests = new String[] { null, ".", "/", "/tmp", "no_such_file_xyz" };
        for (String t : tests) {
            System.out.println("== Java runLs(" + (t == null ? "null" : t) + ") ==");
            runLs(t);
        }
    }
}