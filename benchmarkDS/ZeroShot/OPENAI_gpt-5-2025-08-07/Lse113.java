import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.IDN;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

public class Lse113 {

    public static final class PingResult {
        public final boolean alive;
        public final String pong;
        public final String hostAscii;

        public PingResult(boolean alive, String pong, String hostAscii) {
            this.alive = alive;
            this.pong = pong;
            this.hostAscii = hostAscii;
        }

        @Override
        public String toString() {
            return "alive=" + alive + ", host=" + hostAscii + ", pongLen=" + (pong == null ? 0 : pong.length());
        }
    }

    private static String extractHostAscii(String url) throws Exception {
        if (url == null) throw new IllegalArgumentException("url is null");
        String work = url.trim();
        if (work.isEmpty()) throw new IllegalArgumentException("url is empty");

        String host = null;
        try {
            if (work.contains("://")) {
                URI uri = new URI(work);
                host = uri.getHost();
                if (host == null) {
                    // Fallback: handle cases like http://[::1]
                    String auth = uri.getRawAuthority();
                    if (auth != null) {
                        int at = auth.lastIndexOf('@');
                        if (at >= 0 && at + 1 < auth.length()) auth = auth.substring(at + 1);
                        if (auth.startsWith("[")) {
                            int end = auth.indexOf(']');
                            if (end > 0) host = auth.substring(1, end);
                        } else {
                            int c = auth.indexOf(':');
                            host = c >= 0 ? auth.substring(0, c) : auth;
                        }
                    }
                }
            } else {
                String h = work;
                int sep = h.indexOf('/');
                if (sep >= 0) h = h.substring(0, sep);
                sep = h.indexOf('?');
                if (sep >= 0) h = h.substring(0, sep);
                sep = h.indexOf('#');
                if (sep >= 0) h = h.substring(0, sep);
                if (h.startsWith("[") && h.endsWith("]")) {
                    h = h.substring(1, h.length() - 1);
                }
                host = h;
            }
        } catch (Exception e) {
            throw new IllegalArgumentException("Invalid URL: " + e.getMessage());
        }
        if (host == null || host.isEmpty()) throw new IllegalArgumentException("No host in URL");

        String ascii = IDN.toASCII(host, IDN.ALLOW_UNASSIGNED);
        if (ascii.isEmpty() || ascii.length() > 253) throw new IllegalArgumentException("Invalid host length");
        if (!ascii.matches("^[A-Za-z0-9\\-\\.\\:]+$")) {
            throw new IllegalArgumentException("Host contains invalid characters");
        }
        return ascii;
    }

    public static PingResult pingUrl(String url) {
        String hostAscii;
        try {
            hostAscii = extractHostAscii(url);
        } catch (Exception e) {
            return new PingResult(false, "Error: " + e.getMessage(), null);
        }

        boolean isWindows = System.getProperty("os.name").toLowerCase().contains("win");
        List<String> cmd = new ArrayList<>();
        cmd.add("ping");
        if (isWindows) {
            cmd.add("-n"); cmd.add("1");
            cmd.add("-w"); cmd.add("2000");
        } else {
            cmd.add("-c"); cmd.add("1");
            cmd.add("-W"); cmd.add("2");
        }
        cmd.add(hostAscii);

        StringBuilder out = new StringBuilder();
        int exit = -1;
        Process p = null;
        try {
            ProcessBuilder pb = new ProcessBuilder(cmd);
            pb.redirectErrorStream(true);
            p = pb.start();

            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
                char[] buf = new char[4096];
                int n; int total = 0; int limit = 65536;
                while ((n = br.read(buf)) != -1) {
                    if (total + n > limit) n = Math.max(0, limit - total);
                    if (n == 0) break;
                    out.append(buf, 0, n);
                    total += n;
                }
            }

            boolean finished = p.waitFor(5, TimeUnit.SECONDS);
            if (!finished) {
                p.destroyForcibly();
                out.append("\n[Terminated due to timeout]");
                exit = -1;
            } else {
                exit = p.exitValue();
            }
        } catch (Exception e) {
            out.append("Error executing ping: ").append(e.getMessage());
        } finally {
            if (p != null) p.destroyForcibly();
        }

        boolean alive = (exit == 0);
        return new PingResult(alive, out.toString(), hostAscii);
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "https://example.com",
            "8.8.8.8",
            "example.com",
            "http://[::1]",
            "bad host!!"
        };
        for (String t : tests) {
            PingResult r = pingUrl(t);
            System.out.println("Test: " + t + " -> " + r);
        }
    }
}