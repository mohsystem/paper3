import java.io.IOException;
import java.time.Duration;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

public class Lse110 {
    private static final int INVALID_INPUT_STATUS = 2;
    private static final int TIMEOUT_STATUS = 124;
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-z_][a-z0-9_-]*[$]?$");

    public static int runId(String username) {
        if (!isValidUsername(username)) {
            return INVALID_INPUT_STATUS;
        }

        ProcessBuilder pb = new ProcessBuilder(List.of("id", username));
        Map<String, String> env = pb.environment();
        env.clear();
        env.put("PATH", "/usr/bin:/bin");
        // Avoid inheriting potentially unsafe env vars
        pb.redirectErrorStream(true);

        try {
            Process p = pb.start();
            boolean finished = p.waitFor(5, TimeUnit.SECONDS);
            if (!finished) {
                p.destroyForcibly();
                return TIMEOUT_STATUS;
            }
            return p.exitValue();
        } catch (IOException e) {
            // id not found or cannot execute
            return 127;
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return TIMEOUT_STATUS;
        }
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() == 0 || username.length() > 128) return false;
        return USERNAME_PATTERN.matcher(username.toLowerCase(Locale.ROOT)).matches();
    }

    public static void main(String[] args) {
        if (args.length >= 1) {
            String user = args[0];
            int status = runId(user);
            System.out.println("Exit status for username '" + user + "': " + status);
        } else {
            String[] tests = new String[] {"root", "nobody", "daemon", "syslog", "nonexistentuser12345"};
            for (String u : tests) {
                int status = runId(u);
                System.out.println("Exit status for username '" + u + "': " + status);
            }
        }
    }
}