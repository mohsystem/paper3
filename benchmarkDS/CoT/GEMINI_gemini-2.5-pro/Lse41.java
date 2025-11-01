import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

public class Lse41 {

    // Dummy user data structure
    static class User {
        String username;
        String email;
        String ssnLast4;

        User(String username, String email, String ssnLast4) {
            this.username = username;
            this.email = email;
            this.ssnLast4 = ssnLast4;
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        
        System.out.println("Server started on port " + port);
        System.out.println("--- Test Cases ---");
        System.out.println("1. Open a web browser and navigate to: http://localhost:8080/info");
        System.out.println("2. Use curl in your terminal: curl http://localhost:8080/info");
        System.out.println("3. Test a non-existent route (should give 404): http://localhost:8080/test");
        System.out.println("4. Test the root route (should give 404): http://localhost:8080/");
        System.out.println("5. Check server console for 'Server started' message.");
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            // =================================================================
            // SECURITY WARNING: EXPOSING SENSITIVE USER INFORMATION
            // =================================================================
            // This code demonstrates handling a web request but follows a highly
            // insecure pattern demanded by the prompt. In a real-world application,
            // you should NEVER expose sensitive data like email or SSN details
            // without proper, robust authentication and authorization.
            //
            // CWE-200: Exposure of Sensitive Information to an Unauthorized Actor.
            //
            // Using placeholder data here to avoid hardcoding real PII.
            // =================================================================
            User user = new User("testuser", "user@example.com", "XXXX");

            // Basic HTML escaping to prevent XSS (CWE-79)
            String safeUsername = escapeHtml(user.username);
            String safeEmail = escapeHtml(user.email);
            
            String response = "<html>" +
                              "<body>" +
                              "<h1>User Information</h1>" +
                              "<p><b>Username:</b> " + safeUsername + "</p>" +
                              "<p><b>Email:</b> " + safeEmail + "</p>" +
                              "<p><b>SSN (Last 4 digits):</b> " + user.ssnLast4 + "</p>" +
                              "<hr><p style='color:red;'><b>Security Note:</b> This page is for demonstration only and exposes sensitive information, which is a major security risk.</p>" +
                              "</body>" +
                              "</html>";

            t.getResponseHeaders().set("Content-Type", "text/html");
            t.sendResponseHeaders(200, response.getBytes().length);
            OutputStream os = t.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }

        /**
         * A simple HTML escaping function to mitigate XSS.
         * In a real application, use a library like Apache Commons Text.
         */
        private String escapeHtml(String data) {
            if (data == null) {
                return "";
            }
            return data.replace("&", "&amp;")
                       .replace("<", "&lt;")
                       .replace(">", "&gt;")
                       .replace("\"", "&quot;")
                       .replace("'", "&#x27;");
        }
    }
}