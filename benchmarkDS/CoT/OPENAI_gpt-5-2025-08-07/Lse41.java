import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse41 {

    // 1) Problem understanding:
    // We need to provide a web server with a single route /info that returns a page containing:
    // - username
    // - email
    // - last 4 digits of SSN (never expose full SSN).
    // The core logic is encapsulated in renderInfo(username, email, ssn) that returns HTML string.

    // 2) Security requirements:
    // - No reflection of untrusted input without proper encoding (escape HTML to prevent XSS).
    // - Never expose full SSN; only last 4 digits derived from numeric-only SSN.
    // - Reasonable input validation (email format).
    // - Set basic security headers (CSP, X-Content-Type-Options).
    // - Avoid logging sensitive info.

    // 3) Secure coding generation:
    // Helper to escape HTML special chars to prevent XSS
    public static String escapeHtml(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(s.length() + 16);
        for (char c : s.toCharArray()) {
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

    // Extract last 4 digits from SSN, ensuring only digits are considered
    public static String extractLast4(String ssn) {
        if (ssn == null) return "N/A";
        String digits = ssn.replaceAll("[^0-9]", "");
        if (digits.length() < 4) return "N/A";
        return digits.substring(digits.length() - 4);
    }

    // Basic, conservative email validator. Not exhaustive by RFC, but sufficient here.
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9._%+\\-]+@[A-Za-z0-9.\\-]+\\.[A-Za-z]{2,63}$");
    public static boolean isLikelyValidEmail(String email) {
        if (email == null || email.length() > 254) return false;
        return EMAIL_PATTERN.matcher(email).matches();
    }

    // The required function: accepts inputs and returns the HTML output
    public static String renderInfo(String username, String email, String ssn) {
        String safeUser = escapeHtml(username);
        String safeEmail = escapeHtml(email);
        String last4 = escapeHtml(extractLast4(ssn));
        String emailNote = isLikelyValidEmail(email) ? "" : " (unverified format)";
        String csp = "default-src 'none'; style-src 'unsafe-inline'; img-src 'none'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'";
        // Build a minimal HTML page
        StringBuilder html = new StringBuilder(256);
        html.append("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">")
            .append("<meta http-equiv=\"Content-Security-Policy\" content=\"").append(escapeHtml(csp)).append("\">")
            .append("<meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">")
            .append("<title>User Info</title>")
            .append("<style>body{font-family:sans-serif;margin:2rem;} .box{border:1px solid #ccc;padding:1rem;border-radius:8px;max-width:500px}</style>")
            .append("</head><body>")
            .append("<div class=\"box\">")
            .append("<h1>User Info</h1>")
            .append("<p><strong>Username:</strong> ").append(safeUser).append("</p>")
            .append("<p><strong>Email:</strong> ").append(safeEmail).append(emailNote).append("</p>")
            .append("<p><strong>SSN (last 4):</strong> ").append(last4).append("</p>")
            .append("</div>")
            .append("</body></html>");
        return html.toString();
    }

    // Simple handler for /info
    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                byte[] resp = "Method Not Allowed".getBytes(StandardCharsets.UTF_8);
                exchange.sendResponseHeaders(405, resp.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp);
                }
                return;
            }
            String path = exchange.getRequestURI().getPath();
            if (!"/info".equals(path)) {
                byte[] resp = "Not Found".getBytes(StandardCharsets.UTF_8);
                exchange.sendResponseHeaders(404, resp.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(resp);
                }
                return;
            }

            // Simulated current user (never take SSN via GET parameters)
            String username = "alice";
            String email = "alice@example.com";
            String ssn = "123-45-6789";

            String body = renderInfo(username, email, ssn);
            Headers headers = exchange.getResponseHeaders();
            headers.set("Content-Type", "text/html; charset=UTF-8");
            headers.set("Content-Security-Policy", "default-src 'none'; style-src 'unsafe-inline'; img-src 'none'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'");
            headers.set("X-Content-Type-Options", "nosniff");
            byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(200, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    public static void runServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Java server started on http://localhost:" + port + "/info");
    }

    // 4) Code review: (documented via comments; actual review performed during generation)
    // - All inputs are escaped; SSN only exposes last 4; CSP set; GET only on /info; no reflection of full SSN.

    // 5) Secure code output: Final main with 5 test cases
    public static void main(String[] args) throws Exception {
        // 5 test cases for renderInfo
        String[][] tests = new String[][]{
                {"alice", "alice@example.com", "123-45-6789"},
                {"bob", "bob.smith@corp.co.uk", "000123456"},
                {"<script>alert(1)</script>", "user@ex<ample>.com", "12-34"},
                {"charlie", "invalid-email@", "xxx-yy-7788"},
                {"Δημήτρης", "dimitris@example.gr", "9999"}
        };
        for (int i = 0; i < tests.length; i++) {
            String html = renderInfo(tests[i][0], tests[i][1], tests[i][2]);
            System.out.println("=== Java Test Case " + (i + 1) + " ===");
            System.out.println(html);
            System.out.println();
        }
        // Start server
        runServer(8080);
    }
}