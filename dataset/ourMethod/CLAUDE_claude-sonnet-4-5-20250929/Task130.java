
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Task130 {
    private static final Set<String> ALLOWED_ORIGINS = new HashSet<>(Arrays.asList(
        "https://example.com",
        "https://app.example.com"
    ));
    
    private static final Set<String> ALLOWED_METHODS = new HashSet<>(Arrays.asList(
        "GET", "POST", "OPTIONS"
    ));
    
    private static final Set<String> ALLOWED_HEADERS = new HashSet<>(Arrays.asList(
        "Content-Type", "Authorization"
    ));
    
    private static final int MAX_AGE = 3600;

    static class CORSHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String origin = exchange.getRequestHeaders().getFirst("Origin");
            String method = exchange.getRequestMethod();
            
            if (origin != null && origin.length() <= 256) {
                origin = sanitizeHeader(origin);
                if (ALLOWED_ORIGINS.contains(origin)) {
                    exchange.getResponseHeaders().set("Access-Control-Allow-Origin", origin);
                    exchange.getResponseHeaders().set("Access-Control-Allow-Credentials", "true");
                    exchange.getResponseHeaders().set("Vary", "Origin");
                }
            }
            
            if ("OPTIONS".equals(method)) {
                handlePreflightRequest(exchange);
            } else if (ALLOWED_METHODS.contains(method)) {
                handleActualRequest(exchange, method);
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }
        
        private void handlePreflightRequest(HttpExchange exchange) throws IOException {
            String requestMethod = exchange.getRequestHeaders().getFirst("Access-Control-Request-Method");
            String requestHeaders = exchange.getRequestHeaders().getFirst("Access-Control-Request-Headers");
            
            if (requestMethod != null && requestMethod.length() <= 20) {
                requestMethod = sanitizeHeader(requestMethod);
                if (ALLOWED_METHODS.contains(requestMethod)) {
                    exchange.getResponseHeaders().set("Access-Control-Allow-Methods", 
                        String.join(", ", ALLOWED_METHODS));
                }
            }
            
            if (requestHeaders != null && requestHeaders.length() <= 256) {
                requestHeaders = sanitizeHeader(requestHeaders);
                String[] headers = requestHeaders.split(",");
                boolean allAllowed = true;
                for (String header : headers) {
                    String trimmed = header.trim();
                    if (!ALLOWED_HEADERS.contains(trimmed)) {
                        allAllowed = false;
                        break;
                    }
                }
                if (allAllowed) {
                    exchange.getResponseHeaders().set("Access-Control-Allow-Headers", 
                        String.join(", ", ALLOWED_HEADERS));
                }
            }
            
            exchange.getResponseHeaders().set("Access-Control-Max-Age", String.valueOf(MAX_AGE));
            sendResponse(exchange, 204, "");
        }
        
        private void handleActualRequest(HttpExchange exchange, String method) throws IOException {
            String response = "{\\"status\\":\\"success\\",\\"method\\":\\"" + escapeJson(method) + "\\"}";
            exchange.getResponseHeaders().set("Content-Type", "application/json");
            sendResponse(exchange, 200, response);
        }
        
        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
        
        private String sanitizeHeader(String header) {
            if (header == null) {
                return "";
            }
            return header.replaceAll("[^a-zA-Z0-9:/.\\\\-_,\\\\s]", "");
        }
        
        private String escapeJson(String input) {
            if (input == null) {
                return "";
            }
            return input.replace("\\\\", "\\\\\\\\")\n                       .replace("\\"", "\\\\\\"")
                       .replace("\\n", "\\\\n")
                       .replace("\\r", "\\\\r");
        }
    }

    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/api/data", new CORSHandler());
        server.setExecutor(null);
        server.start();
        
        System.out.println("Test Case 1: Server started on port 8080");
        System.out.println("Test Case 2: Allowed origins: " + ALLOWED_ORIGINS);
        System.out.println("Test Case 3: Allowed methods: " + ALLOWED_METHODS);
        System.out.println("Test Case 4: Allowed headers: " + ALLOWED_HEADERS);
        System.out.println("Test Case 5: Max age: " + MAX_AGE + " seconds");
        
        Thread.sleep(5000);
        server.stop(0);
        System.out.println("Server stopped");
    }
}
