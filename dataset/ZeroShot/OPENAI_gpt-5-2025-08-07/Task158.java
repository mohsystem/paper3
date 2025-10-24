import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.TimeUnit;

public class Task158 {
    private static final int OUTPUT_LIMIT = 8192;
    private static final int TIMEOUT_SECONDS = 5;

    public static String runPing(String userInput) {
        if (!isValidHost(userInput)) {
            return "Invalid input";
        }
        boolean isWindows = System.getProperty("os.name").toLowerCase().contains("win");
        String[] cmd = isWindows
                ? new String[] { "ping", "-n", "1", userInput }
                : new String[] { "ping", "-c", "1", userInput };
        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        Process proc = null;
        try {
            proc = pb.start();
            try (InputStream in = new BufferedInputStream(proc.getInputStream())) {
                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                byte[] buf = new byte[1024];
                long deadline = System.nanoTime() + TimeUnit.SECONDS.toNanos(TIMEOUT_SECONDS);
                while (System.nanoTime() < deadline) {
                    while (in.available() > 0) {
                        int toRead = Math.min(in.available(), buf.length);
                        int read = in.read(buf, 0, toRead);
                        if (read < 0) break;
                        int remaining = OUTPUT_LIMIT - bos.size();
                        if (remaining <= 0) break;
                        bos.write(buf, 0, Math.min(read, remaining));
                    }
                    if (proc.waitFor(100, TimeUnit.MILLISECONDS)) break;
                }
                if (proc.isAlive()) {
                    proc.destroyForcibly();
                    return "Timed out";
                }
                return bos.toString(StandardCharsets.UTF_8.name());
            }
        } catch (Exception e) {
            if (proc != null) proc.destroyForcibly();
            return "Execution failed";
        }
    }

    private static boolean isValidHost(String s) {
        if (s == null) return false;
        if (s.length() < 1 || s.length() > 253) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '.' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "localhost",
            "127.0.0.1",
            "example.com",
            "bad;rm -rf /",
            "name with spaces"
        };
        for (String t : tests) {
            String out = runPing(t);
            System.out.println("Input: " + t);
            System.out.println("Output (truncated):");
            System.out.println(out);
            System.out.println("----");
        }
    }
}