import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.concurrent.Executors;

// Main class for the web application
public class Lse41 {

    // A simple class to represent a user's data
    private static class User {
        private final String username;
        private final String email;
        private final String socialSecurityNumber; // Store the full SSN securely in a real app

        public User(String username, String email, String ssn) {
            this.username = username;
            this.email = email;
            this.socialSecurityNumber = ssn;
        }

        public String getUsername() {
            return username;
        }

        public String getEmail() {
            return email;
        }

        // Method to get only the last 4 digits of the SSN
        public String getLast4Ssn() {
            if (socialSecurityNumber != null && socialSecurityNumber.length() > 4) {
                return socialSecurityNumber.substring(socialSecurityNumber.length() - 4);
            }
            return "****"; // Return a masked value if SSN is invalid
        }
    }

    // A utility method to escape HTML to prevent XSS (Cross-Site Scripting) attacks.
    private static String escapeHtml(String text) {
        if (text == null) {
            return "";
        }
        return text.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\"", "&quot;")
                   .replace("'", "&#x27;");
    }

    // Handler for the /info route
    static class InfoHandler implements HttpHandler {
        private final User currentUser;

        public InfoHandler(User user) {
            this.currentUser = user;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // In a real application, you would first check if the user is authenticated and authorized.
            // For this example, we assume the user is logged in and represented by `currentUser`.
            
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            // Securely escape user data before including it in the HTML response.
            String safeUsername = escapeHtml(currentUser.getUsername());
            String safeEmail = escapeHtml(currentUser.getEmail());
            String last4Ssn = currentUser.getLast4Ssn(); // Already safe as it's just digits

            String response = "<html>" +
                              "<head><title>User Information</title></head>" +
                              "<body>" +
                              "<h1>User Information</h1>" +
                              "<p><strong>Username:</strong> " + safeUsername + "</p>" +
                              "<p><strong>Email:</strong> " + safeEmail + "</p>" +
                              "<p><strong>Last 4 of SSN:</strong> ****-**-" + last4Ssn + "</p>" +
                              "</body>" +
                              "</html>";

            sendHtmlResponse(exchange, 200, response);
        }

        private void sendHtmlResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            // Set secure headers. In a real application, you'd add more, like CSP.
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
        
        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
             exchange.sendResponseHeaders(statusCode, response.getBytes().length);
             try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }
    
    // Handler for all other routes to return a 404 Not Found
    static class NotFoundHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "404 Not Found";
            exchange.sendResponseHeaders(404, response.length());
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }


    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);

        // --- Test Cases ---
        // For a web server, test cases involve creating different "users"
        // and setting up the server. To test, run the code and visit the URL.
        
        // Test Case 1: Standard User
        User user1 = new User("john.doe", "john.doe@example.com", "123-45-6789");
        // Test Case 2: User with special characters in name that need escaping
        User user2 = new User("<script>alert('xss')</script>", "attacker@example.com", "987-65-4321");
        // Test Case 3: User with a short SSN
        User user3 = new User("jane.doe", "jane.doe@example.com", "555");
        // Test Case 4: User with null values
        User user4 = new User(null, "no-name@example.com", null);
        // Test Case 5: A different user
        User user5 = new User("test_user_5", "test5@example.com", "111-22-3333");
        
        // We will run the server with user from Test Case 1.
        // To test other cases, change `user1` to `user2`, `user3`, etc. below.
        User currentUserForServer = user1;

        // Create the context for the /info URL
        server.createContext("/info", new InfoHandler(currentUserForServer));
        
        // A default context for any other request to return 404
        server.createContext("/", new NotFoundHandler());

        // Use a fixed thread pool for handling requests
        server.setExecutor(Executors.newFixedThreadPool(10));
        server.start();

        System.out.println("Server started on port " + port);
        System.out.println("--- Testing Instructions ---");
        System.out.println("1. Open a web browser.");
        System.out.println("2. Visit: http://localhost:" + port + "/info");
        System.out.println("   -> Expected: Page with info for user: " + currentUserForServer.getUsername());
        System.out.println("3. Visit: http://localhost:" + port + "/otherpage");
        System.out.println("   -> Expected: 404 Not Found");
        System.out.println("\n--- Important Security Note ---");
        System.out.println("This is a simple HTTP server. For production, always use HTTPS to encrypt data in transit.");
    }
}