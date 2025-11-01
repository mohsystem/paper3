import spark.Spark;
import java.util.regex.Pattern;

public class Lse132 {

    // A simple HTML escaper to prevent XSS.
    // In a real application, use a library like Apache Commons Text.
    private static String escapeHtml(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\"", "&quot;")
                    .replace("'", "&#39;");
    }

    // This function is bound to the route /hello/<username>
    public static String hello(spark.Request req, spark.Response res) {
        String username = req.params(":username");

        // Rule #1: Validate input
        // Username should be alphanumeric + underscore, 1 to 50 chars.
        Pattern usernamePattern = Pattern.compile("^[a-zA-Z0-9_]{1,50}$");
        if (username == null || !usernamePattern.matcher(username).matches()) {
            Spark.halt(400, "Invalid username format.");
            return null; // Unreachable, but required by compiler
        }
        
        // Sanitize output for HTML context to prevent XSS
        String sanitizedUsername = escapeHtml(username);

        // This simulates rendering a template file.
        // For a real application, use a template engine like Thymeleaf or FreeMarker.
        String template = "<html><body><h1>Hello, %s!</h1></body></html>";
        
        res.type("text/html");
        return String.format(template, sanitizedUsername);
    }

    public static void main(String[] args) {
        // Set a port
        Spark.port(4567);

        // A decorator is used to bind a function to a URL.
        // In SparkJava, this is achieved by passing a method reference.
        Spark.get("/hello/:username", Lse132::hello);

        System.out.println("Server started on http://localhost:4567");
        System.out.println("Try the following 5 test cases in your browser or with curl:");
        System.out.println("1. Valid: http://localhost:4567/hello/Alice");
        System.out.println("2. Valid with numbers: http://localhost:4567/hello/user123");
        System.out.println("3. Valid with underscore: http://localhost:4567/hello/test_user");
        System.out.println("4. Invalid characters: http://localhost:4567/hello/bad!user (should return 400 Bad Request)");
        System.out.println("5. XSS attempt: http://localhost:4567/hello/<script>alert(1)</script> (should display escaped text)");
    }
}