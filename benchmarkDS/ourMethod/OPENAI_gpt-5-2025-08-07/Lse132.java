import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream; 
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.Objects;
import java.util.regex.Pattern;

public class Lse132 {

    // Security: restrict username to safe characters and reasonable length.
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_-]{1,32}$");

    // Create a hello.html template in a secure temporary directory.
    public static Path createTemplateFile() throws IOException {
        String template = "<!doctype html>\n" +
                "<html>\n" +
                "<head><meta charset=\"utf-8\"><title>Hello</title></head>\n" +
                "<body>\n" +
                "  <h1>Hello, {{ username }}</h1>\n" +
                "</body>\n" +
                "</html>\n";
        Path dir = Files.createTempDirectory("lse132_templates_");
        Path file = dir.resolve("hello.html");
        Files.write(file, template.getBytes(StandardCharsets.UTF_8),
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
        return file;
    }

    // Validate username input
    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    // Simple HTML escaping to prevent XSS
    public static String escapeHtml(String s) {
        StringBuilder sb = new StringBuilder(s.length() + 16);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#x27;"); break;
                case '/': sb.append("&#x2F;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    // Render the hello template with the username injected safely.
    public static String renderHello(String username, Path templatePath) throws IOException {
        Objects.requireNonNull(templatePath, "templatePath");
        if (!isValidUsername(username)) {
            return "Invalid username";
        }
        byte[] data = Files.readAllBytes(templatePath);
        String template = new String(data, StandardCharsets.UTF_8);
        String safeUser = escapeHtml(username);
        String placeholder = "{{ username }}";
        return template.replace(placeholder, safeUser);
    }

    // Start a minimal HTTP server to serve /hello/<username>
    public static HttpServer startServer(Path templatePath, int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/hello", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                try {
                    String method = exchange.getRequestMethod();
                    if (!"GET".equalsIgnoreCase(method)) {
                        exchange.sendResponseHeaders(405, -1);
                        return;
                    }
                    String path = exchange.getRequestURI().getPath(); // e.g., /hello/alice
                    String base = "/hello/";
                    if (!path.startsWith(base) || path.length() <= base.length()) {
                        byte[] msg = "Bad Request".getBytes(StandardCharsets.UTF_8);
                        exchange.sendResponseHeaders(400, msg.length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(msg);
                        }
                        return;
                    }
                    String username = path.substring(base.length());
                    if (!isValidUsername(username)) {
                        byte[] msg = "Invalid username".getBytes(StandardCharsets.UTF_8);
                        exchange.sendResponseHeaders(400, msg.length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(msg);
                        }
                        return;
                    }
                    String body = renderHello(username, templatePath);
                    byte[] resp = body.getBytes(StandardCharsets.UTF_8);
                    Headers h = exchange.getResponseHeaders();
                    h.set("Content-Type", "text/html; charset=UTF-8");
                    exchange.sendResponseHeaders(200, resp.length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(resp);
                    }
                } finally {
                    exchange.close();
                }
            }
        });
        server.setExecutor(null);
        server.start();
        return server;
    }

    public static void main(String[] args) throws Exception {
        Path tmpl = createTemplateFile();

        // 5 test cases
        String[] tests = new String[] { "Alice", "bob_123", "user-01", "X", "Z9_9-_" };
        for (String t : tests) {
            String out = renderHello(t, tmpl);
            System.out.println("---- Test: " + t + " ----");
            System.out.println(out);
        }

        // Optionally start server (commented to avoid long-running process in test environments)
        // HttpServer server = startServer(tmpl, 8080);
        // System.out.println("Server started at http://127.0.0.1:8080/hello/<username>");
        // System.out.println("Press Enter to stop.");
        // new BufferedReader(new InputStreamReader(System.in)).readLine();
        // server.stop(0);
    }
}