// Chain-of-Through process in code generation:
// 1) Problem understanding: We need to safely use user-provided input as part of an OS command.
//    We'll implement a safe ping function that executes the OS "ping" command without invoking a shell.
//    It accepts a hostname/IP as input and returns the command output. We'll include a main with 5 test cases.
// 2) Security requirements: Prevent command injection and resource abuse.
//    - Do not use a shell; use ProcessBuilder with argument arrays.
//    - Strictly validate input (only allow valid hostnames or IPv4).
//    - Enforce execution timeouts and limit output size.
// 3) Secure coding generation: Implement the function with validation, least privilege, timeouts,
//    and careful output handling. No environment mutation.
// 4) Code review: Ensure no shell usage, strict regex validation, timeouts, safe stream handling,
//    and robust error handling.
// 5) Secure code output: Final code below reflects mitigations above.

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

public class Task158 {

    // Validate hostname (RFC 1035-ish) and IPv4 strictly. No spaces or special shell chars.
    private static final Pattern HOSTNAME_PATTERN = Pattern.compile(
            "^(?=.{1,253}$)([A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)(?:\\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*$"
    );
    private static final Pattern IPV4_PATTERN = Pattern.compile(
            "^((25[0-5]|2[0-4]\\d|1?\\d?\\d)(\\.|$)){4}$"
    );

    private static boolean isValidHostOrIPv4(String host) {
        if (host == null) return false;
        String h = host.trim();
        if (h.isEmpty()) return false;
        if (h.length() > 253) return false;
        // Quick character allowlist to prevent weird unicode or shell metacharacters.
        for (int i = 0; i < h.length(); i++) {
            char c = h.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
                    || (c >= '0' && c <= '9') || c == '-' || c == '.';
            if (!ok) return false;
        }
        return HOSTNAME_PATTERN.matcher(h).matches() || IPV4_PATTERN.matcher(h).matches();
    }

    // Safe ping function: never invokes a shell; uses argument array; validates input and enforces timeout.
    public static String safePing(String host, Duration timeout) throws IOException, InterruptedException {
        if (!isValidHostOrIPv4(host)) {
            return "Invalid host input rejected.";
        }
        boolean isWindows = System.getProperty("os.name").toLowerCase().contains("win");
        List<String> cmd = new ArrayList<>();
        if (isWindows) {
            // ping -n 1 -w <ms> host
            cmd.add("ping");
            cmd.add("-n");
            cmd.add("1");
            cmd.add("-w");
            long ms = Math.min(Math.max(timeout.toMillis(), 500), 10000); // clamp 0.5s..10s
            cmd.add(Long.toString(ms));
            cmd.add(host);
        } else {
            // ping -c 1 -W <sec> host (Linux); on macOS -W is in ms, but we also use process timeout
            cmd.add("ping");
            cmd.add("-c");
            cmd.add("1");
            long sec = Math.min(Math.max(timeout.toSeconds(), 1), 10); // clamp 1..10
            cmd.add("-W");
            cmd.add(Long.toString(sec));
            cmd.add(host);
        }

        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        Process p = pb.start();

        StringBuilder out = new StringBuilder();
        // Read output concurrently with process execution to avoid buffer deadlock
        Thread reader = new Thread(() -> {
            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
                char[] buf = new char[4096];
                int n;
                int total = 0;
                int max = 64 * 1024; // cap output collection to 64KB
                while ((n = br.read(buf)) != -1) {
                    if (total + n > max) {
                        out.append(buf, 0, Math.max(0, max - total));
                        break;
                    } else {
                        out.append(buf, 0, n);
                        total += n;
                    }
                }
            } catch (IOException ignored) {
            }
        });
        reader.setDaemon(true);
        reader.start();

        boolean finished = p.waitFor(timeout.toMillis(), TimeUnit.MILLISECONDS);
        if (!finished) {
            p.destroyForcibly();
            reader.join(200);
            return "Command timed out.";
        }
        reader.join(200);
        int code = p.exitValue();
        String output = out.toString().trim();
        if (output.isEmpty()) output = (code == 0 ? "No output." : "No output, exit=" + code);
        return output;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] tests = new String[] {
                "localhost",
                "127.0.0.1",
                "example.com",
                "invalid host", // should be rejected
                "8.8.8.8"
        };
        for (String t : tests) {
            try {
                String res = safePing(t, Duration.ofSeconds(3));
                System.out.println("Test host: " + t);
                System.out.println(res);
                System.out.println("----");
            } catch (Exception e) {
                System.out.println("Test host: " + t);
                System.out.println("Error: " + e.getMessage());
                System.out.println("----");
            }
        }
    }
}