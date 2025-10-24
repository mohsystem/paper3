
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Task38 {
    private static final int MAX_INPUT_LENGTH = 1000;
    private static final Pattern SAFE_INPUT_PATTERN = Pattern.compile("^[a-zA-Z0-9\\\\s.,!?'-]*$");
    
    public static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        
        if (input.length() > MAX_INPUT_LENGTH) {
            input = input.substring(0, MAX_INPUT_LENGTH);
        }
        
        if (!SAFE_INPUT_PATTERN.matcher(input).matches()) {
            return "";
        }
        
        return escapeHtml(input);
    }
    
    public static String escapeHtml(String input) {
        if (input == null) {
            return "";
        }
        
        StringBuilder escaped = new StringBuilder();
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '&':
                    escaped.append("&amp;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\\'':
                    escaped.append("&#x27;");
                    break;
                default:
                    escaped.append(c);
            }
        }
        return escaped.toString();
    }
    
    public static String processUserInput(String userInput) {
        return sanitizeInput(userInput);
    }
    
    public static void main(String[] args) {
        System.out.println("Test 1: " + processUserInput("Hello World"));
        System.out.println("Test 2: " + processUserInput("Test123"));
        System.out.println("Test 3: " + processUserInput("<script>alert('xss')</script>"));
        System.out.println("Test 4: " + processUserInput("Valid input with spaces!"));
        System.out.println("Test 5: " + processUserInput("a".repeat(1500)));
        
        try {
            HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
            
            server.createContext("/", new HttpHandler() {
                @Override
                public void handle(HttpExchange exchange) throws IOException {
                    String response = "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\"><title>Form</title></head><body>" +
                        "<h1>User Input Form</h1>" +
                        "<form method=\\"POST\\" action=\\"/submit\\">" +
                        "<label>Enter text:</label><br>" +
                        "<input type=\\"text\\" name=\\"userInput\\" maxlength=\\"1000\\" required><br><br>" +
                        "<input type=\\"submit\\" value=\\"Submit\\">" +
                        "</form></body></html>";
                    
                    exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                    exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(response.getBytes(StandardCharsets.UTF_8));
                    }
                }
            });
            
            server.createContext("/submit", new HttpHandler() {
                @Override
                public void handle(HttpExchange exchange) throws IOException {
                    if ("POST".equals(exchange.getRequestMethod())) {
                        try (InputStream is = exchange.getRequestBody()) {
                            byte[] buffer = new byte[MAX_INPUT_LENGTH + 100];
                            int bytesRead = is.read(buffer);
                            if (bytesRead > 0) {
                                String body = new String(buffer, 0, bytesRead, StandardCharsets.UTF_8);
                                Map<String, String> params = parseFormData(body);
                                String userInput = params.getOrDefault("userInput", "");
                                String sanitized = sanitizeInput(userInput);
                                
                                String response = "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\"><title>Result</title></head><body>" +
                                    "<h1>Your Input:</h1><p>" + sanitized + "</p>" +
                                    "<a href=\\"/\\">Back to form</a></body></html>";
                                
                                exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                                exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                                try (OutputStream os = exchange.getResponseBody()) {
                                    os.write(response.getBytes(StandardCharsets.UTF_8));
                                }
                            }
                        }
                    }
                }
            });
            
            server.setExecutor(null);
            server.start();
            System.out.println("Server started on port 8080");
        } catch (IOException e) {
            System.err.println("Server error: " + e.getMessage());
        }
    }
    
    private static Map<String, String> parseFormData(String formData) {
        Map<String, String> params = new HashMap<>();
        if (formData == null || formData.isEmpty()) {
            return params;
        }
        
        String[] pairs = formData.split("&");
        for (String pair : pairs) {
            String[] keyValue = pair.split("=", 2);
            if (keyValue.length == 2) {
                try {
                    String key = URLDecoder.decode(keyValue[0], StandardCharsets.UTF_8.name());
                    String value = URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8.name());
                    params.put(key, value);
                } catch (Exception e) {
                    // Skip invalid pairs
                }
            }
        }
        return params;
    }
}
