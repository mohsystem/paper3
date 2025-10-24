
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.security.SecureRandom;
import java.util.Base64;

public class Task38 {
    private static Map<String, String> csrfTokens = new HashMap<>();
    private static SecureRandom random = new SecureRandom();

    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new FormHandler());
        server.createContext("/submit", new SubmitHandler());
        server.setExecutor(null);
        System.out.println("Server started on port 8080");
        server.start();
    }

    static class FormHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String csrfToken = generateCSRFToken();
            csrfTokens.put(csrfToken, "valid");
            
            String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>" +
                "<title>Secure Form</title></head><body>" +
                "<h1>User Input Form</h1>" +
                "<form method='POST' action='/submit'>" +
                "<input type='hidden' name='csrf_token' value='" + escapeHtml(csrfToken) + "'>" +
                "<label>Name: <input type='text' name='name' maxlength='100' required></label><br>" +
                "<label>Email: <input type='email' name='email' maxlength='100' required></label><br>" +
                "<label>Message: <textarea name='message' maxlength='500' required></textarea></label><br>" +
                "<input type='submit' value='Submit'>" +
                "</form></body></html>";
            
            sendResponse(exchange, 200, html);
        }
    }

    static class SubmitHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
            BufferedReader br = new BufferedReader(isr);
            String formData = br.readLine();
            
            Map<String, String> params = parseFormData(formData);
            
            // CSRF validation
            String csrfToken = params.get("csrf_token");
            if (csrfToken == null || !csrfTokens.containsKey(csrfToken)) {
                sendResponse(exchange, 403, "Invalid CSRF token");
                return;
            }
            csrfTokens.remove(csrfToken);
            
            // Input validation and sanitization
            String name = sanitizeInput(params.get("name"), 100);
            String email = sanitizeInput(params.get("email"), 100);
            String message = sanitizeInput(params.get("message"), 500);
            
            if (!isValidEmail(email)) {
                sendResponse(exchange, 400, "Invalid email format");
                return;
            }
            
            String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>" +
                "<title>Form Submission Result</title></head><body>" +
                "<h1>Your Submitted Data</h1>" +
                "<p><strong>Name:</strong> " + escapeHtml(name) + "</p>" +
                "<p><strong>Email:</strong> " + escapeHtml(email) + "</p>" +
                "<p><strong>Message:</strong> " + escapeHtml(message) + "</p>" +
                "<a href='/'>Back to Form</a></body></html>";
            
            sendResponse(exchange, 200, html);
        }
    }

    private static String generateCSRFToken() {
        byte[] bytes = new byte[32];
        random.nextBytes(bytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
    }

    private static Map<String, String> parseFormData(String formData) {
        Map<String, String> params = new HashMap<>();
        if (formData != null) {
            for (String param : formData.split("&")) {
                String[] pair = param.split("=", 2);
                if (pair.length == 2) {
                    try {
                        params.put(URLDecoder.decode(pair[0], "UTF-8"), 
                                  URLDecoder.decode(pair[1], "UTF-8"));
                    } catch (Exception e) {}
                }
            }
        }
        return params;
    }

    private static String sanitizeInput(String input, int maxLength) {
        if (input == null) return "";
        input = input.trim();
        if (input.length() > maxLength) {
            input = input.substring(0, maxLength);
        }
        return input;
    }

    private static String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }

    private static boolean isValidEmail(String email) {
        return email != null && email.matches("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
    }

    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
        exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
        exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
        exchange.getResponseHeaders().set("X-XSS-Protection", "1; mode=block");
        exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
        OutputStream os = exchange.getResponseBody();
        os.write(response.getBytes(StandardCharsets.UTF_8));
        os.close();
    }
}
