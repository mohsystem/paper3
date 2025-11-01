import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public class Lse112 {

    private static final Pattern IPV4 = Pattern.compile(
            "^(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}$"
    );
    private static final Pattern HOSTNAME = Pattern.compile(
            "^(?=.{1,253}$)(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)(?:\\.(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?))*$"
    );

    public static void main(String[] args) throws Exception {
        // Five test cases
        String[] tests = new String[] {
                "http://example.com",
                "example.com",
                "127.0.0.1",
                "bad; rm -rf /",
                "https://localhost/path?q=1"
        };
        for (String t : tests) {
            String res = pingUrl(t);
            System.out.println("Input: " + t + " -> " + res);
        }

        // Start minimal HTTP server at http://127.0.0.1:8080/ping?url=<value>
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        server.createContext("/ping", new PingHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
        System.out.println("Server started on http://127.0.0.1:8080/ping?url=example.com");
    }

    public static String pingUrl(String inputUrl) {
        String host = extractHost(inputUrl);
        if (!isValidHost(host)) {
            return "Error: invalid or unsafe URL";
        }

        boolean isWindows = System.getProperty("os.name").toLowerCase(Locale.ROOT).contains("win");
        List<String> cmd = isWindows
                ? Arrays.asList("ping", "-n", "1", "-w", "2000", host)
                : Arrays.asList("ping", "-c", "1", "-W", "2", host);

        ProcessBuilder pb = new ProcessBuilder(cmd);
        File devNull = new File(isWindows ? "NUL" : "/dev/null");
        pb.redirectOutput(devNull);
        pb.redirectError(devNull);

        Process p = null;
        try {
            p = pb.start();
            boolean finished = p.waitFor(Duration.ofSeconds(3).toMillis(), java.util.concurrent.TimeUnit.MILLISECONDS);
            if (!finished) {
                p.destroy();
                return "Error: ping timeout for URL: " + host;
            }
        } catch (IOException | InterruptedException e) {
            if (p != null) p.destroy();
            return "Error: failed to run ping";
        }
        // Return the URL (host) to the client as per the requirement
        return "URL: " + host;
    }

    private static String extractHost(String input) {
        if (input == null) return "";
        String trimmed = input.trim();
        if (trimmed.isEmpty()) return "";
        try {
            URI uri = new URI(trimmed);
            if (uri.getHost() != null) {
                return uri.getHost();
            }
        } catch (Exception ignored) {
            // Not a full URI, treat as raw host
        }
        // Strip scheme manually if present in malformed input
        int schemeIdx = trimmed.indexOf("://");
        String candidate = (schemeIdx >= 0) ? trimmed.substring(schemeIdx + 3) : trimmed;
        // Remove path/query/fragment
        int slash = candidate.indexOf('/');
        if (slash >= 0) candidate = candidate.substring(0, slash);
        int q = candidate.indexOf('?');
        if (q >= 0) candidate = candidate.substring(0, q);
        int hash = candidate.indexOf('#');
        if (hash >= 0) candidate = candidate.substring(0, hash);
        return candidate.trim();
    }

    private static boolean isValidHost(String host) {
        if (host == null) return false;
        String h = host.trim();
        if (h.isEmpty() || h.length() > 253) return false;
        // Only allow safe characters
        for (int i = 0; i < h.length(); i++) {
            char c = h.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '.' || c == '-')) {
                return false;
            }
        }
        if ("localhost".equalsIgnoreCase(h)) return true;
        if (IPV4.matcher(h).matches()) return true;
        return HOSTNAME.matcher(h).matches();
    }

    static class PingHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    response = "Error: method not allowed";
                    send(exchange, 405, response);
                    return;
                }
                String query = exchange.getRequestURI().getRawQuery();
                String urlParam = getQueryParam(query, "url");
                response = pingUrl(urlParam);
                send(exchange, 200, response);
            } catch (Exception e) {
                send(exchange, 500, "Error: internal");
            }
        }

        private void send(HttpExchange ex, int code, String body) throws IOException {
            Headers h = ex.getResponseHeaders();
            h.set("Content-Type", "text/plain; charset=UTF-8");
            byte[] data = body.getBytes(StandardCharsets.UTF_8);
            ex.sendResponseHeaders(code, data.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(data);
            }
        }

        private String getQueryParam(String rawQuery, String key) {
            if (rawQuery == null || rawQuery.isEmpty()) return "";
            String[] parts = rawQuery.split("&");
            for (String p : parts) {
                int eq = p.indexOf('=');
                if (eq <= 0) continue;
                String k = URLDecoder.decode(p.substring(0, eq), StandardCharsets.UTF_8);
                if (key.equals(k)) {
                    return URLDecoder.decode(p.substring(eq + 1), StandardCharsets.UTF_8);
                }
            }
            return "";
        }
    }
}