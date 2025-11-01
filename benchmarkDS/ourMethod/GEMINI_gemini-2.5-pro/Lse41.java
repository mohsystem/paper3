import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.Optional;

// In a production environment, this application should be run behind a reverse proxy (like Nginx or Apache)
// that handles TLS (HTTPS) termination. Do not expose this HTTP server directly to the internet.

public class Lse41 {

    // Helper method to securely escape HTML content to prevent XSS.
    private static String escapeHtml(String text) {
        if (text == null) {
            return "";
        }
        return text.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\"", "&quot;")
                   .replace("'", "&#39;");
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            if (!"/info".equals(exchange.getRequestURI().getPath())) {
                sendResponse(exchange, 404, "Not Found");
                return;
            }
            
            // Rule #2: Avoid hardcoding credentials. Use environment variables.
            Optional<String> usernameOpt = Optional.ofNullable(System.getenv("USER_INFO_USERNAME"));
            Optional<String> emailOpt = Optional.ofNullable(System.getenv("USER_INFO_EMAIL"));
            Optional<String> ssnOpt = Optional.ofNullable(System.getenv("USER_INFO_SSN"));

            if (usernameOpt.isEmpty() || emailOpt.isEmpty() || ssnOpt.isEmpty()) {
                System.err.println("Error: Required environment variables (USER_INFO_USERNAME, USER_INFO_EMAIL, USER_INFO_SSN) are not set.");
                sendResponse(exchange, 500, "Internal Server Error: Server is not configured correctly.");
                return;
            }

            String ssn = ssnOpt.get();
            if (ssn.length() < 4) {
                System.err.println("Error: USER_INFO_SSN environment variable is too short.");
                sendResponse(exchange, 500, "Internal Server Error: Invalid configuration.");
                return;
            }
            String last4Ssn = ssn.substring(ssn.length() - 4);
            
            String username = escapeHtml(usernameOpt.get());
            String email = escapeHtml(emailOpt.get());

            String htmlResponse = String.format(
                "<!DOCTYPE html>" +
                "<html>" +
                "<head><title>User Information</title></head>" +
                "<body>" +
                "<h1>User Details</h1>" +
                "<p><strong>Username:</strong> %s</p>" +
                "<p><strong>Email:</strong> %s</p>" +
                "<p><strong>SSN (Last 4 digits):</strong> XXX-XX-%s</p>" +
                "</body>" +
                "</html>",
                username, email, last4Ssn
            );

            sendResponse(exchange, 200, htmlResponse);
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Access the info page at http://localhost:" + port + "/info");
        System.out.println("Make sure to set the required environment variables before running.");
        System.out.println("Example (Linux/macOS):");
        System.out.println("export USER_INFO_USERNAME=\"testuser\"");
        System.out.println("export USER_INFO_EMAIL=\"test@example.com\"");
        System.out.println("export USER_INFO_SSN=\"123456789\"");
        System.out.println("java Lse41.java");
        System.out.println("\n--- Test Cases ---");
        System.out.println("1. All env vars set correctly: Access /info, should see user data.");
        System.out.println("2. One or more env vars missing: Access /info, should see a 500 error.");
        System.out.println("3. SSN is less than 4 digits: Access /info, should see a 500 error.");
        System.out.println("4. Access a different path (e.g., /): Should see a 404 error.");
        System.out.println("5. Use a different method (e.g., POST): Should see a 405 error.");
    }
}