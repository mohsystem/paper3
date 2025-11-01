import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ConcurrentHashMap;

public class Lse149 {

    // In-memory "database" using a thread-safe list for concurrent access.
    private static final List<Map<String, String>> messages = new CopyOnWriteArrayList<>();

    /**
     * Simulates inserting a user message into the database.
     * @param username The name of the user.
     * @param message The message content.
     */
    public static void insert_user_message_in_db(String username, String message) {
        if (username != null && !username.trim().isEmpty() && message != null && !message.trim().isEmpty()) {
            Map<String, String> entry = new ConcurrentHashMap<>();
            entry.put("username", username);
            entry.put("message", message);
            messages.add(entry);
        }
    }

    public static void main(String[] args) throws IOException {
        // 5 Test cases: Pre-populate the database
        insert_user_message_in_db("Alice", "Hello from Java!");
        insert_user_message_in_db("Bob", "This is a native Java HTTP server.");
        insert_user_message_in_db("Charlie", "No external frameworks needed.");
        insert_user_message_in_db("David", "HttpServer is part of the JDK.");
        insert_user_message_in_db("Eve", "Let's test the submission form.");

        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/post", new PostHandler());
        server.setExecutor(null); // creates a default executor
        System.out.println("Java server started on port 8000. Open http://localhost:8000/");
        server.start();
    }

    // Handler for the root URL "/"
    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            StringBuilder response = new StringBuilder();
            response.append("<html><body>");
            response.append("<h1>Post a Message</h1>");
            response.append("<form action=\"/post\" method=\"post\">");
            response.append("Username: <input type=\"text\" name=\"username\"><br>");
            response.append("Message:  <input type=\"text\" name=\"message\"><br>");
            response.append("<input type=\"submit\" value=\"Post\">");
            response.append("</form>");
            response.append("<hr><h1>Messages</h1>");
            response.append("<ul>");

            // Display messages in reverse chronological order
            for (int i = messages.size() - 1; i >= 0; i--) {
                Map<String, String> msg = messages.get(i);
                String username = escapeHtml(msg.get("username"));
                String message = escapeHtml(msg.get("message"));
                response.append("<li><b>").append(username).append(":</b> ").append(message).append("</li>");
            }

            response.append("</ul></body></html>");

            byte[] responseBytes = response.toString().getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(200, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }

    // Handler for the "/post" URL
    static class PostHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
                BufferedReader br = new BufferedReader(isr);
                String formData = br.readLine(); // e.g., username=test&message=hello

                Map<String, String> params = parseFormData(formData);
                String username = params.get("username");
                String message = params.get("message");

                insert_user_message_in_db(username, message);

                // Redirect user to the root page
                exchange.getResponseHeaders().set("Location", "/");
                exchange.sendResponseHeaders(302, -1); // 302 Found (for redirection)
            } else {
                exchange.sendResponseHeaders(405, -1); // 405 Method Not Allowed
            }
        }

        private Map<String, String> parseFormData(String formData) {
            Map<String, String> parameters = new ConcurrentHashMap<>();
            if (formData != null) {
                String[] pairs = formData.split("&");
                for (String pair : pairs) {
                    String[] keyValue = pair.split("=", 2);
                    if (keyValue.length == 2) {
                        try {
                            String key = URLDecoder.decode(keyValue[0], StandardCharsets.UTF_8.name());
                            String value = URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8.name());
                            parameters.put(key, value);
                        } catch (java.io.UnsupportedEncodingException e) {
                            // This should not happen with UTF-8
                        }
                    }
                }
            }
            return parameters;
        }
    }

    // Basic HTML escaping to prevent Cross-Site Scripting (XSS)
    private static String escapeHtml(String text) {
        if (text == null) return "";
        return text.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\"", "&quot;")
                   .replace("'", "&#039;");
    }
}