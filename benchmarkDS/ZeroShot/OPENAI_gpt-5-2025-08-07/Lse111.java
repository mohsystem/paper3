import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public class Lse111 {

    // Validate host: allow IPv4, basic hostname, or IPv6 (colon present and ASCII)
    private static final Pattern IPV4 = Pattern.compile("^(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}$");
    private static final Pattern HOSTNAME = Pattern.compile("^(?=.{1,253}$)(?!-)[A-Za-z0-9-]{1,63}(?<!-)(\\.(?!-)[A-Za-z0-9-]{1,63}(?<!-))*\\.?$");
    private static final Pattern ALLOWED_CHARS = Pattern.compile("^[A-Za-z0-9.:-]+$");

    private static boolean isValidHost(String host) {
        if (host == null) return false;
        host = host.trim();
        if (host.isEmpty() || host.length() > 253) return false;
        if (!ALLOWED_CHARS.matcher(host).matches()) return false;
        if (IPV4.matcher(host).matches()) return true;
        if (HOSTNAME.matcher(host).matches()) return true;
        if (host.contains(":")) {
            // Basic IPv6 sanity check using Java's parser without DNS lookup side-effects
            try {
                InetAddress addr = InetAddress.getByName(host);
                return addr instanceof Inet6Address || host.contains(":");
            } catch (Exception e) {
                return false;
            }
        }
        return false;
    }

    public static String ping(String url) {
        String host = url == null ? "" : url.trim();
        if (!isValidHost(host)) {
            return "Invalid host";
        }
        boolean isWindows = System.getProperty("os.name").toLowerCase(Locale.ROOT).contains("win");
        List<String> cmd = new ArrayList<>();
        if (isWindows) {
            cmd = Arrays.asList("ping", "-n", "2", "-w", "2000", host);
        } else {
            cmd = Arrays.asList("ping", "-c", "2", "-W", "2", host);
        }

        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        StringBuilder out = new StringBuilder();
        Process p = null;
        try {
            p = pb.start();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
                char[] buf = new char[4096];
                int n;
                long deadline = System.nanoTime() + Duration.ofSeconds(8).toNanos();
                while ((n = br.read(buf)) != -1) {
                    out.append(buf, 0, n);
                    if (out.length() > 100000) { // safety cap
                        break;
                    }
                    if (System.nanoTime() > deadline) {
                        break;
                    }
                }
            }
            if (!p.waitFor(8, java.util.concurrent.TimeUnit.SECONDS)) {
                p.destroyForcibly();
                out.append("\n[Timed out]");
            }
        } catch (IOException e) {
            return "Error executing ping: " + e.getMessage();
        } catch (InterruptedException ie) {
            Thread.currentThread().interrupt();
            return "Interrupted";
        } finally {
            if (p != null && p.isAlive()) {
                p.destroyForcibly();
            }
        }
        return out.toString();
    }

    // Simple HTTP server for /ping?url=...
    private static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/ping", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    exchange.sendResponseHeaders(405, -1);
                    return;
                }
                String query = exchange.getRequestURI().getRawQuery();
                String urlParam = "";
                if (query != null) {
                    for (String kv : query.split("&")) {
                        int i = kv.indexOf('=');
                        if (i >= 0) {
                            String k = URLDecoder.decode(kv.substring(0, i), StandardCharsets.UTF_8);
                            if ("url".equals(k)) {
                                urlParam = URLDecoder.decode(kv.substring(i + 1), StandardCharsets.UTF_8);
                                break;
                            }
                        }
                    }
                }
                String result = ping(urlParam);
                byte[] bytes = result.getBytes(StandardCharsets.UTF_8);
                exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
                exchange.sendResponseHeaders(200, bytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(bytes);
                }
            }
        });
        server.setExecutor(Executors.newFixedThreadPool(2));
        server.start();
    }

    public static void main(String[] args) throws Exception {
        // Start server in background
        new Thread(() -> {
            try {
                startServer(8080);
            } catch (IOException e) {
                // ignore in tests
            }
        }).start();

        // 5 test cases
        List<String> tests = Arrays.asList(
                "localhost",
                "127.0.0.1",
                "::1",
                "example.com",
                "invalid_host!"
        );
        for (String t : tests) {
            System.out.println("== ping(" + t + ") ==");
            System.out.println(ping(t));
        }
    }
}