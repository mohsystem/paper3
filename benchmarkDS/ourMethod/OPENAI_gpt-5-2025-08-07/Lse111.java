import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class Lse111 {

    // Extracts host from a URL string or returns the raw input if it looks like a host.
    private static String extractHost(String inputUrl) {
        if (inputUrl == null) {
            return null;
        }
        inputUrl = inputUrl.trim();
        if (inputUrl.length() == 0 || inputUrl.length() > 2048) {
            return null;
        }
        try {
            URI uri = new URI(inputUrl);
            String host = uri.getHost();
            if (host == null) {
                // Handle URLs like http://[::1] where URI may not parse host as expected or raw host inputs
                String s = inputUrl;
                int schemeIdx = s.indexOf("://");
                int start = (schemeIdx >= 0) ? schemeIdx + 3 : 0;
                int end = s.length();
                for (int i = start; i < s.length(); i++) {
                    char c = s.charAt(i);
                    if (c == '/' || c == '?' || c == '#') {
                        end = i;
                        break;
                    }
                }
                host = s.substring(start, end);
                if (host.startsWith("[") && host.endsWith("]")) {
                    host = host.substring(1, host.length() - 1);
                }
            }
            if (host != null) {
                host = host.trim();
            }
            return (host != null && !host.isEmpty()) ? host : null;
        } catch (Exception e) {
            // Treat input as a raw host if it does not parse as a URI
            String host = inputUrl;
            if (host.startsWith("[") && host.endsWith("]")) {
                host = host.substring(1, host.length() - 1);
            }
            return (host.length() > 0) ? host : null;
        }
    }

    // Basic validation for hostname/IPv4/IPv6 literals
    private static boolean isValidHost(String host) {
        if (host == null) return false;
        if (host.length() == 0 || host.length() > 253) return false;
        // Disallow leading/trailing dots/hyphens and consecutive dots
        if (host.startsWith(".") || host.endsWith(".") || host.startsWith("-") || host.endsWith("-")) return false;
        if (host.contains("..")) return false;

        // Allow only safe characters for hostname or IP literals
        for (int i = 0; i < host.length(); i++) {
            char c = host.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z')
                    || (c >= 'a' && c <= 'z')
                    || (c >= '0' && c <= '9')
                    || c == '.' || c == '-' || c == ':';
            if (!ok) return false;
        }

        // If contains colon, treat as possible IPv6; allow hex digits and colons
        if (host.indexOf(':') >= 0) {
            // A very basic sanity check for IPv6 length and characters
            // We avoid DNS lookups for validation here.
            int colons = 0;
            for (int i = 0; i < host.length(); i++) {
                char c = host.charAt(i);
                if (c == ':') colons++;
                else {
                    boolean hx = (c >= '0' && c <= '9') ||
                                 (c >= 'a' && c <= 'f') ||
                                 (c >= 'A' && c <= 'F') ||
                                 c == '.';
                    if (!hx) return false;
                }
            }
            return colons >= 2; // minimal indicator it's likely IPv6
        }

        // Otherwise hostname/IPv4 style
        return true;
    }

    public static String ping(String inputUrl) {
        String host = extractHost(inputUrl);
        if (!isValidHost(host)) {
            return "Invalid host";
        }

        boolean isWindows = System.getProperty("os.name").toLowerCase().contains("win");
        List<String> cmd = new ArrayList<>();
        cmd.add("ping");
        if (isWindows) {
            // Windows: -n count, -w timeout(ms)
            cmd.add("-n");
            cmd.add("1");
            cmd.add("-w");
            cmd.add("1000");
        } else {
            // Unix-like: -n numeric, -c count, -W timeout(seconds)
            cmd.add("-n");
            cmd.add("-c");
            cmd.add("1");
            cmd.add("-W");
            cmd.add("1");
        }
        cmd.add(host);

        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);

        StringBuilder sb = new StringBuilder();
        final int MAX_OUT = 65536;
        Process p = null;
        try {
            p = pb.start();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
                char[] buf = new char[1024];
                int r;
                while ((r = br.read(buf)) != -1) {
                    if (sb.length() + r > MAX_OUT) {
                        sb.append(buf, 0, Math.max(0, MAX_OUT - sb.length()));
                        break;
                    } else {
                        sb.append(buf, 0, r);
                    }
                }
            }
            if (!p.waitFor(5, TimeUnit.SECONDS)) {
                p.destroyForcibly();
                if (sb.length() == 0) {
                    return "Ping timed out";
                }
            }
        } catch (IOException ioe) {
            return "Ping failed";
        } catch (InterruptedException ie) {
            Thread.currentThread().interrupt();
            return "Ping interrupted";
        } finally {
            if (p != null) p.destroy();
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "http://127.0.0.1",
            "https://localhost",
            "https://example.com",
            "http://[::1]/",
            "http://bad_host_$$$"
        };
        for (String t : tests) {
            System.out.println("=== Test: " + t + " ===");
            String out = ping(t);
            System.out.println(out.length() > 500 ? out.substring(0, 500) + "...(truncated)" : out);
            System.out.println();
        }
    }
}