import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

public class Task120 {

    /**
     * Escapes special characters in a string for use in HTML.
     * This is crucial to prevent Cross-Site Scripting (XSS) attacks.
     *
     * @param input The raw string to be escaped.
     * @return The HTML-escaped string.
     */
    public static String htmlEscape(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder escaped = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '&':
                    escaped.append("&amp;");
                    break;
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\'':
                    escaped.append("&#39;");
                    break;
                default:
                    escaped.append(c);
                    break;
            }
        }
        return escaped.toString();
    }

    /**
     * Generates an HTML page displaying the user-provided (and now escaped) input.
     *
     * @param userInput The user-provided string.
     * @return A full HTML page as a string.
     */
    public static String generateWebPage(String userInput) {
        String escapedInput = htmlEscape(userInput);
        return "<!DOCTYPE html>\n" +
               "<html>\n" +
               "<head>\n" +
               "    <title>User Input Display</title>\n" +
               "    <meta charset=\"UTF-8\">\n" +
               "</head>\n" +
               "<body>\n" +
               "    <h1>User-Supplied Input:</h1>\n" +
               "    <p style=\"border: 1px solid black; padding: 10px; background-color: #f0f0f0;\">\n" +
               "        " + escapedInput + "\n" +
               "    </p>\n" +
               "</body>\n" +
               "</html>";
    }

    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String query = t.getRequestURI().getQuery();
            Map<String, String> params = parseQuery(query);
            String userInput = params.getOrDefault("input", "No input provided.");
            
            String response = generateWebPage(userInput);
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);

            t.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            t.sendResponseHeaders(200, responseBytes.length);
            try (OutputStream os = t.getResponseBody()) {
                os.write(responseBytes);
            }
        }

        private Map<String, String> parseQuery(String query) {
            Map<String, String> result = new HashMap<>();
            if (query != null && !query.isEmpty()) {
                for (String param : query.split("&")) {
                    String[] pair = param.split("=", 2);
                    if (pair.length > 1) {
                        try {
                            String key = URLDecoder.decode(pair[0], StandardCharsets.UTF_8.name());
                            String value = URLDecoder.decode(pair[1], StandardCharsets.UTF_8.name());
                            result.put(key, value);
                        } catch (java.io.UnsupportedEncodingException e) {
                            // This should not happen with UTF-8
                        }
                    }
                }
            }
            return result;
        }
    }

    public static void main(String[] args) {
        // --- Test cases for the core logic ---
        System.out.println("--- Testing generateWebPage Function ---");
        
        // Test Case 1: Simple text
        String input1 = "Hello, world!";
        System.out.println("Test Case 1 (Simple Text):\n" + generateWebPage(input1) + "\n");
        
        // Test Case 2: Text with HTML characters
        String input2 = "<p>This is a paragraph.</p>";
        System.out.println("Test Case 2 (HTML Chars):\n" + generateWebPage(input2) + "\n");
        
        // Test Case 3: XSS attack attempt
        String input3 = "<script>alert('XSS');</script>";
        System.out.println("Test Case 3 (XSS Attempt):\n" + generateWebPage(input3) + "\n");
        
        // Test Case 4: Text with quotes and ampersand
        String input4 = "AT&T's new motto is \"Rethink Possible\".";
        System.out.println("Test Case 4 (Quotes & Ampersand):\n" + generateWebPage(input4) + "\n");
        
        // Test Case 5: Empty input
        String input5 = "";
        System.out.println("Test Case 5 (Empty Input):\n" + generateWebPage(input5) + "\n");

        // --- Optional: Start a web server to see it in a browser ---
        try {
            int port = 8080;
            HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
            server.createContext("/", new MyHandler());
            server.setExecutor(Executors.newSingleThreadExecutor());
            server.start();
            System.out.println("--- Server started on port " + port + " ---");
            System.out.println("Try accessing: http://localhost:8080/?input=Your%20Text%20Here");
            System.out.println("Or for an XSS test: http://localhost:8080/?input=%3Cscript%3Ealert(1)%3C/script%3E");
        } catch (IOException e) {
            System.err.println("Failed to start server: " + e.getMessage());
        }
    }
}