import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

public class Lse115 {

    // Build a formatted string similar to printf, returning the result
    public static String sprint(String fmt, Object... args) {
        if (fmt == null) throw new IllegalArgumentException("Format string cannot be null");
        return String.format(Locale.ROOT, fmt, args);
    }

    // Validate path to avoid dangerous characters and option injection
    private static boolean isSafePath(String s) {
        if (s == null) return false;
        if (s.isEmpty()) return false;
        if (s.length() > 4096) return false;
        if (s.charAt(0) == '-') return false; // avoid option injection
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            // allow alnum, space, '/', '.', '_', '-', and common unicode safe characters will be rejected
            if (!(Character.isLetterOrDigit(c) || c == ' ' || c == '/' || c == '.' || c == '_' || c == '-')) {
                return false;
            }
        }
        return true;
    }

    // Execute ls safely without invoking a shell; returns combined stdout/stderr
    public static String executeLs(String arg) throws IOException, InterruptedException {
        String path = (arg == null || arg.isEmpty()) ? "." : arg;
        if (!isSafePath(path)) {
            return "Invalid or unsafe path input.\n";
        }

        ProcessBuilder pb = new ProcessBuilder("ls", "--color=never", "--", path);
        pb.redirectErrorStream(true); // merge stderr into stdout
        Process p = pb.start();

        StringBuilder out = new StringBuilder();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
            char[] buf = new char[8192];
            int n;
            // Limit size to prevent excessive memory use
            long max = 10 * 1024 * 1024L; // 10 MB
            long total = 0;
            while ((n = br.read(buf)) != -1) {
                total += n;
                if (total > max) {
                    out.append("\n[Output truncated]\n");
                    break;
                }
                out.append(buf, 0, n);
            }
        }
        p.waitFor();
        return out.toString();
    }

    public static void main(String[] args) throws Exception {
        String[] tests = new String[] {
            sprint("%s", "."),
            sprint("%s", "/"),
            sprint("%s", ".."),
            sprint("./%s", "nonexistent_dir_hopefully"),
            sprint("%s-%s", "src", "main")
        };
        for (int i = 0; i < tests.length; i++) {
            String input = tests[i];
            String result = executeLs(input);
            System.out.println("Test " + (i + 1) + " input: " + input);
            System.out.println(result);
            System.out.println("----");
        }
    }
}