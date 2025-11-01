import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Lse133 {

    // Function that accepts input as parameter and returns output
    public static String buildMessage(String name) {
        String n = (name == null) ? "" : name.trim();
        if (n.isEmpty()) {
            n = "World";
        }
        if (!isValidName(n)) {
            throw new IllegalArgumentException("Invalid name");
        }
        return "Hello, " + n + "!";
    }

    private static boolean isValidName(String s) {
        if (s == null) return false;
        if (s.length() < 1 || s.length() > 50) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = Character.isLetterOrDigit(c) || c == ' ' || c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    private static String getQueryParam(String query, String key) {
        if (query == null) return null;
        String[] pairs = query.split("&");
        for (String p : pairs) {
            int eq = p.indexOf('=');
            String k = eq >= 0 ? p.substring(0, eq) : p;
            String v = eq >= 0 ? p.substring(eq + 1) : "";
            if (k.equals(key)) {
                try {
                    return URLDecoder.decode(v, StandardCharsets.UTF_8.name());
                } catch (IllegalArgumentException e) {
                    return null;
                } catch (Exception e) {
                    return null;
                }
            }
        }
        return null;
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        String[] tests = new String[] { "World", "Alice", "Bob_12", "Eve-Ok", " " };
        for (String t : tests) {
            try {
                System.out.println(buildMessage(t));
            } catch (Exception ex) {
                System.out.println("Error: " + ex.getMessage());
            }
        }

        // Create and run a minimal HTTP application securely on localhost
        InetSocketAddress addr = new InetSocketAddress("127.0.0.1", 8080);
        HttpServer server = HttpServer.create(addr, 0);
        ExecutorService pool = Executors.newFixedThreadPool(4);
        server.setExecutor(pool);

        server.createContext("/", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                try {
                    if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                        exchange.sendResponseHeaders(405, -1);
                        return;
                    }
                    String name = getQueryParam(exchange.getRequestURI().getQuery(), "name");
                    String msg;
                    try {
                        msg = buildMessage(name);
                    } catch (IllegalArgumentException e) {
                        String body = "Invalid name";
                        Headers h = exchange.getResponseHeaders();
                        h.set("Content-Type", "text/plain; charset=utf-8");
                        exchange.sendResponseHeaders(400, body.getBytes(StandardCharsets.UTF_8).length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(body.getBytes(StandardCharsets.UTF_8));
                        }
                        return;
                    }
                    Headers h = exchange.getResponseHeaders();
                    h.set("Content-Type", "text/plain; charset=utf-8");
                    byte[] body = msg.getBytes(StandardCharsets.UTF_8);
                    exchange.sendResponseHeaders(200, body.length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(body);
                    }
                } finally {
                    exchange.close();
                }
            }
        });

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            server.stop(0);
            pool.shutdown();
        }));

        server.start();
    }
}