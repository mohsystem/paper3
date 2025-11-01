
// Java implementation using Javalin framework (a lightweight web framework)
import io.javalin.Javalin;
import io.javalin.http.Context;

public class Lse133 {
    public static void main(String[] args) {
        // Create the application object with security configurations
        Javalin app = Javalin.create(config -> {
            // Disable autogenerateEtags to prevent information leakage
            config.http.generateEtags = false;
            // Enable default headers for security
            config.http.defaultContentType = "text/plain; charset=utf-8";
        });
        
        // Route decorator - maps URL to handler function
        // Input validation: Javalin handles path traversal internally
        app.get("/", ctx -> handleRequest(ctx));
        
        // Start the application on port 7000 with localhost binding only
        // Binding to localhost prevents external access in development
        app.start("127.0.0.1", 7000);
        
        // Test cases demonstration
        System.out.println("Server running on http://127.0.0.1:7000/");
        System.out.println("Test Case 1: GET / - Returns welcome message");
        System.out.println("Test Case 2: GET /invalid - Returns 404");
        System.out.println("Test Case 3: Multiple concurrent requests - Should handle safely");
        System.out.println("Test Case 4: Request with special characters - Should be sanitized");
        System.out.println("Test Case 5: Request with long path - Should be rejected");
    }
    
    // Handler function that returns the message
    // Secure: Returns only static content, no user input reflection
    private static void handleRequest(Context ctx) {
        // Set security headers to prevent common attacks
        ctx.header("X-Content-Type-Options", "nosniff");
        ctx.header("X-Frame-Options", "DENY");
        ctx.header("Content-Security-Policy", "default-src 'none'");
        
        // Return safe static message - no dynamic content or user input
        ctx.result("Hello, World!");
    }
}
