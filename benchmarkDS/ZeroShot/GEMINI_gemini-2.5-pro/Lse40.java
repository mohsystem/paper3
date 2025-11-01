import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpSession;

@SpringBootApplication
@RestController
class Lse40 {

    // NOTE: To run this Java code, you need to have a Spring Boot environment set up.
    // This is a single-file Spring Boot application. You can run it if you have the Spring
    // Boot CLI, or by setting up a Maven/Gradle project with the 'spring-boot-starter-web' dependency.

    /**
     * Simulates a user login by storing user details in the session.
     * In a real application, this would be a POST request and would involve
     * proper authentication.
     *
     * @param username The user's name.
     * @param email The user's email.
     * @param ssn The user's full Social Security Number.
     * @param request The HttpServletRequest to get the session from.
     * @return A confirmation message.
     */
    @GetMapping("/login")
    public String login(@RequestParam String username, @RequestParam String email, @RequestParam String ssn, HttpServletRequest request) {
        // Basic input validation
        if (username == null || username.trim().isEmpty() || 
            email == null || email.trim().isEmpty() || 
            ssn == null || !ssn.matches("\\d{9}")) {
            return "Invalid input provided for login.";
        }

        HttpSession session = request.getSession(true);
        session.setAttribute("username", username);
        session.setAttribute("email", email);
        session.setAttribute("ssn", ssn); // Storing full SSN in session is risky, ensure session is secure.
        return "User '" + username + "' logged in successfully. Session created.";
    }

    /**
     * Displays user information stored in the session.
     *
     * @param request The HttpServletRequest to get the session from.
     * @return An HTML page with user info or an error message.
     */
    @GetMapping("/info")
    public ResponseEntity<String> info(HttpServletRequest request) {
        HttpSession session = request.getSession(false); // false = do not create new session

        if (session == null || session.getAttribute("username") == null) {
            return new ResponseEntity<>("<h1>Unauthorized</h1><p>Please log in first.</p>", HttpStatus.UNAUTHORIZED);
        }

        String username = (String) session.getAttribute("username");
        String email = (String) session.getAttribute("email");
        String ssn = (String) session.getAttribute("ssn");

        String last4Ssn = "invalid";
        if (ssn != null && ssn.length() > 4) {
            last4Ssn = ssn.substring(ssn.length() - 4);
        }

        // Basic HTML escaping for security, although data comes from the session.
        // In a real app with templates (like Thymeleaf), this is handled automatically.
        String safeUsername = escapeHtml(username);
        String safeEmail = escapeHtml(email);

        String htmlResponse = "<html><head><title>User Info</title></head><body>"
                            + "<h1>User Information</h1>"
                            + "<p><strong>Username:</strong> " + safeUsername + "</p>"
                            + "<p><strong>Email:</strong> " + safeEmail + "</p>"
                            + "<p><strong>Last 4 digits of SSN:</strong> ****-**-" + last4Ssn + "</p>"
                            + "</body></html>";

        return new ResponseEntity<>(htmlResponse, HttpStatus.OK);
    }
    
    /**
     * A simple utility to escape HTML characters and prevent XSS.
     */
    private String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\"", "&quot;")
                    .replace("'", "&#39;");
    }

    public static void main(String[] args) {
        // This main method starts the web server.
        SpringApplication.run(Lse40.class, args);
        
        System.out.println("Server started on port 8080.");
        System.out.println("Use a tool like curl or your web browser to test the endpoints.");
        System.out.println("\n--- 5 Test Cases ---");
        System.out.println("NOTE: Use a new terminal for these commands. The '-c cookie.txt' and '-b cookie.txt' flags manage the session cookie.");
        
        System.out.println("\n1. Access /info without logging in (should be unauthorized):");
        System.out.println("curl -v http://localhost:8080/info");

        System.out.println("\n2. Log in as 'testuser1':");
        System.out.println("curl -c cookie.txt \"http://localhost:8080/login?username=testuser1&email=test1@example.com&ssn=123456789\"");
        
        System.out.println("\n3. Access /info as 'testuser1' (should succeed):");
        System.out.println("curl -b cookie.txt http://localhost:8080/info");
        
        System.out.println("\n4. Log in as a different user, 'another_user', in the same session:");
        System.out.println("curl -c cookie.txt -b cookie.txt \"http://localhost:8080/login?username=another_user&email=another@example.com&ssn=987654321\"");
        
        System.out.println("\n5. Access /info again (should show 'another_user' data):");
        System.out.println("curl -b cookie.txt http://localhost:8080/info");
    }
}