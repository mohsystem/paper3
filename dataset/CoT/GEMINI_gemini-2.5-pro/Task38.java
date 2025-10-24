import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Task38 {

    public static void main(String[] args) throws IOException {
        int port = 8000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new RootHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Open http://localhost:8000 in your browser.");
        System.out.println("\n--- Manual Test Cases ---");
        System.out.println("1. Enter a simple name like 'Alice' and submit.");
        System.out.println("2. Enter a name with spaces like 'Bob Smith' and submit.");
        System.out.println("3. Enter text with special characters like '1 < 5 & 4 > 3' and submit.");
        System.out.println("4. Enter an XSS payload like '<script>alert(\"xss\")</script>' and verify it is displayed as text, not executed.");
        System.out.println("5. Submit the form with an empty input field.");
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equals(exchange.getRequestMethod())) {
                handleGetRequest(exchange);
            } else if ("POST".equals(exchange.getRequestMethod())) {
                handlePostRequest(exchange);
            }
        }

        private void handleGetRequest(HttpExchange exchange) throws IOException {
            String formHtml = "<html>" +
                              "<head><title>User Input Form</title></head>" +
                              "<body>" +
                              "<h2>Enter your name:</h2>" +
                              "<form method='post' action='/'>" +
                              "<input type='text' name='username' autofocus>" +
                              "<input type='submit' value='Submit'>" +
                              "</form>" +
                              "</body>" +
                              "</html>";
            sendResponse(exchange, 200, formHtml);
        }

        private void handlePostRequest(HttpExchange exchange) throws IOException {
            InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
            BufferedReader br = new BufferedReader(isr);
            String formData = br.lines().collect(Collectors.joining("\n"));
            
            Map<String, String> params = parseFormData(formData);
            String username = params.getOrDefault("username", "");

            // Security: Escape HTML to prevent XSS attacks
            String sanitizedUsername = escapeHtml(username);

            String responseHtml = "<html>" +
                                  "<head><title>User Input Display</title></head>" +
                                  "<body>" +
                                  "<h2>Hello, " + sanitizedUsername + "!</h2>" +
                                  "<a href='/'>Go back</a>" +
                                  "</body>" +
                                  "</html>";
            sendResponse(exchange, 200, responseHtml);
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }

        private Map<String, String> parseFormData(String formData) {
            Map<String, String> parameters = new HashMap<>();
            if (formData == null || formData.isEmpty()) {
                return parameters;
            }
            String[] pairs = formData.split("&");
            for (String pair : pairs) {
                int idx = pair.indexOf("=");
                try {
                    if (idx > 0) {
                        String key = URLDecoder.decode(pair.substring(0, idx), "UTF-8");
                        String value = URLDecoder.decode(pair.substring(idx + 1), "UTF-8");
                        parameters.put(key, value);
                    }
                } catch (java.io.UnsupportedEncodingException e) {
                    // This should not happen with UTF-8
                    e.printStackTrace();
                }
            }
            return parameters;
        }

        /**
         * Escapes characters that have a special meaning in HTML.
         * This is a crucial security measure to prevent Cross-Site Scripting (XSS).
         */
        public static String escapeHtml(String text) {
            if (text == null) {
                return "";
            }
            return text.replace("&", "&amp;")
                       .replace("<", "&lt;")
                       .replace(">", "&gt;")
                       .replace("\"", "&quot;")
                       .replace("'", "&#39;");
        }
    }
}