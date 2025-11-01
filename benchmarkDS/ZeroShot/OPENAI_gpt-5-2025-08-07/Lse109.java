import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.*;

public class Lse109 {

    // Validate username to avoid injection; allow typical username chars only
    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() == 0 || username.length() > 32) return false;
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.' )) {
                return false;
            }
        }
        return true;
    }

    // Execute a two-step pipeline: cmd1 | cmd2, returns stdout of cmd2 as String
    public static String execPipeline(List<String> cmd1, List<String> cmd2) throws IOException, InterruptedException, TimeoutException {
        Process p1 = new ProcessBuilder(cmd1).redirectErrorStream(true).start();
        Process p2 = new ProcessBuilder(cmd2).redirectErrorStream(true).start();

        // Pipe p1 stdout to p2 stdin
        Thread pipeThread = new Thread(() -> {
            try (InputStream in = p1.getInputStream(); OutputStream out = p2.getOutputStream()) {
                byte[] buf = new byte[8192];
                int r;
                while ((r = in.read(buf)) != -1) {
                    out.write(buf, 0, r);
                }
                out.flush();
            } catch (IOException ignored) { }
        });
        pipeThread.start();

        ByteArrayOutputStream result = new ByteArrayOutputStream();
        Future<?> readerFuture = Executors.newSingleThreadExecutor().submit(() -> {
            try (InputStream in = p2.getInputStream()) {
                byte[] buf = new byte[8192];
                int r;
                while ((r = in.read(buf)) != -1) {
                    result.write(buf, 0, r);
                }
            } catch (IOException ignored) { }
        });

        // Wait with timeout
        long timeoutMs = 5000;
        long start = System.currentTimeMillis();

        pipeThread.join(timeoutMs);
        if (pipeThread.isAlive()) throw new TimeoutException("Pipeline stalled");
        long elapsed = System.currentTimeMillis() - start;

        if (!p1.waitFor(Math.max(1, (int)((timeoutMs - elapsed) / 1000)), java.util.concurrent.TimeUnit.SECONDS)) {
            p1.destroyForcibly();
            throw new TimeoutException("p1 timeout");
        }
        elapsed = System.currentTimeMillis() - start;
        try {
            readerFuture.get(Math.max(1, timeoutMs - elapsed), TimeUnit.MILLISECONDS);
        } catch (Exception e) {
            throw new TimeoutException("reader timeout");
        }
        if (!p2.waitFor(Math.max(1, (int)((timeoutMs - (System.currentTimeMillis() - start)) / 1000)), java.util.concurrent.TimeUnit.SECONDS)) {
            p2.destroyForcibly();
            throw new TimeoutException("p2 timeout");
        }

        return result.toString(StandardCharsets.UTF_8);
    }

    // Get user info using secure pipeline: /bin/cat /etc/passwd | /bin/grep -F username
    public static String getUserInfo(String username) {
        if (!isValidUsername(username)) {
            return "";
        }
        List<String> cmd1 = Arrays.asList("/bin/cat", "/etc/passwd");
        List<String> cmd2 = Arrays.asList("/bin/grep", "-F", username);
        try {
            return execPipeline(cmd1, cmd2);
        } catch (Exception e) {
            return "";
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] tests = new String[] {
            args != null && args.length > 0 ? args[0] : "root",
            "daemon",
            "bin",
            "nosuchuser",
            "invalid;rm"
        };
        for (String u : tests) {
            String out = getUserInfo(u);
            System.out.println("Query: " + u);
            System.out.print(out);
            if (out.isEmpty()) {
                System.out.println("(no output or invalid username)");
            }
            System.out.println("-----");
        }
    }
}