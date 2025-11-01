/*
NOTE: To compile and run this Java code, you need the Spark Java framework.
You can add it to your project using a build tool like Maven or Gradle.

Maven dependency:
<dependency>
    <groupId>com.sparkjava</groupId>
    <artifactId>spark-core</artifactId>
    <version>2.9.4</version>
</dependency>

To run, execute the main method. It will start a server on port 4567
and then run 5 test cases against a simulated request handler.
*/
import spark.Request;
import spark.Response;
import spark.Route;
import spark.Session;
import static spark.Spark.*;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.util.HashMap;
import java.util.Map;

// Main class name as per instruction
class Lse42 {

    /**
     * SECURITY WARNING: This code is for demonstration purposes only.
     * Exposing sensitive Personally Identifiable Information (PII) like a Social Security Number (SSN)
     * via an unauthenticated API endpoint is extremely insecure and should NEVER be done in a real application.
     * Real applications must have strong authentication, authorization, and data encryption.
     * The SSN should also be masked or only parts of it displayed if absolutely necessary.
     */
    public static String info(Request req, Response res) {
        // Use req.session(true) to create a session if one doesn't exist.
        Session session = req.session(true);

        // Retrieve user data from the session. Use a default value if an attribute is missing.
        String username = session.attribute("username");
        String email = session.attribute("email");
        String ssn = session.attribute("ssn");

        // Secure Coding: Check if the user is actually logged in by verifying a key attribute.
        if (username == null || username.trim().isEmpty()) {
            res.status(401); // Unauthorized
            return "{\"error\": \"User not authenticated. Please log in.\"}";
        }

        // Secure Coding: Avoid returning nulls in the output.
        username = (username != null) ? username : "N/A";
        email = (email != null) ? email : "N/A";
        ssn = (ssn != null) ? ssn : "N/A"; // In a real app, this should be masked.

        res.type("application/json");
        return String.format("{\"username\": \"%s\", \"email\": \"%s\", \"ssn\": \"%s\"}",
                username, email, ssn);
    }

    // A mock Request class for testing purposes
    static class MockRequest extends Request {
        private final Session session;

        public MockRequest(Map<String, Object> sessionAttributes) {
            this.session = new MockSession(sessionAttributes);
        }

        @Override
        public Session session(boolean create) {
            return this.session;
        }

        @Override
        public Session session() {
            return this.session;
        }
    }

    // A mock Session class for testing purposes
    static class MockSession extends Session {
        private final Map<String, Object> attributes;

        public MockSession(Map<String, Object> attributes) {
            this.attributes = (attributes != null) ? attributes : new HashMap<>();
        }

        @Override
        public <T> T attribute(String name) {
            return (T) attributes.get(name);
        }

        @Override
        public void attribute(String name, Object value) {
            attributes.put(name, value);
        }

        @Override
        public void removeAttribute(String name) {
            attributes.remove(name);
        }
    }

    // Test runner function
    public static void runTests() {
        System.out.println("--- Running Local Handler Tests ---");
        Response mockResponse = new Response() {};

        // Test Case 1: All user data is present in the session.
        System.out.println("\n--- Test Case 1: Valid Session ---");
        Map<String, Object> session1 = new HashMap<>();
        session1.put("username", "jdoe");
        session1.put("email", "jdoe@example.com");
        session1.put("ssn", "**********"); // Masked for security
        Request req1 = new MockRequest(session1);
        String result1 = info(req1, mockResponse);
        System.out.println("Expected: Contains jdoe, jdoe@example.com, **********");
        System.out.println("Actual: " + result1);

        // Test Case 2: Some data is missing (ssn).
        System.out.println("\n--- Test Case 2: Session with Missing Data ---");
        Map<String, Object> session2 = new HashMap<>();
        session2.put("username", "asmith");
        session2.put("email", "asmith@example.com");
        Request req2 = new MockRequest(session2);
        String result2 = info(req2, mockResponse);
        System.out.println("Expected: Contains asmith, asmith@example.com, N/A");
        System.out.println("Actual: " + result2);

        // Test Case 3: User is not logged in (username is missing).
        System.out.println("\n--- Test Case 3: Unauthenticated Session ---");
        Map<String, Object> session3 = new HashMap<>();
        session3.put("email", "hacker@example.com");
        session3.put("ssn", "**********");
        Request req3 = new MockRequest(session3);
        String result3 = info(req3, mockResponse);
        System.out.println("Expected: Contains 'User not authenticated'");
        System.out.println("Actual: " + result3);

        // Test Case 4: Session is completely empty.
        System.out.println("\n--- Test Case 4: Empty Session ---");
        Request req4 = new MockRequest(new HashMap<>());
        String result4 = info(req4, mockResponse);
        System.out.println("Expected: Contains 'User not authenticated'");
        System.out.println("Actual: " + result4);

        // Test Case 5: Different valid user.
        System.out.println("\n--- Test Case 5: Different Valid User ---");
        Map<String, Object> session5 = new HashMap<>();
        session5.put("username", "test.user");
        session5.put("email", "test.user@example.com");
        session5.put("ssn", "**********");
        Request req5 = new MockRequest(session5);
        String result5 = info(req5, mockResponse);
        System.out.println("Expected: Contains test.user, test.user@example.com, **********");
        System.out.println("Actual: " + result5);
        System.out.println("--- Local Handler Tests Complete ---");
    }


    public static void main(String[] args) {
        // Run local tests before starting the server
        runTests();

        // Start the web server
        port(4567);
        System.out.println("\nStarting server at http://localhost:4567");
        System.out.println("Access http://localhost:4567/info to see the result.");
        System.out.println("NOTE: The server will show an authentication error because a session is not established via a login route.");

        // Define the /info route for the web application
        get("/info", (req, res) -> info(req, res));
    }
}