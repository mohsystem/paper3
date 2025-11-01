/*
NOTE: This code requires the SparkJava library. It cannot be run as a single file 
without a build tool like Maven or Gradle to manage dependencies.

Example Maven dependency:
<dependencies>
    <dependency>
        <groupId>com.sparkjava</groupId>
        <artifactId>spark-core</artifactId>
        <version>2.9.4</version>
    </dependency>
</dependencies>
*/
import static spark.Spark.*;

public class Lse133 {

    // This is the function associated with the URL route.
    public static String handleHelloRequest() {
        // The returned string is static and contains no user input,
        // so it is safe from injection attacks like XSS.
        return "Hello, World!";
    }

    public static void runTests() {
        System.out.println("--- Running Java Tests ---");
        // Test 1: Handler returns the correct string
        if ("Hello, World!".equals(handleHelloRequest())) {
            System.out.println("Test 1 Passed: Handler returns correct message.");
        } else {
            System.out.println("Test 1 Failed.");
        }
        
        // Test 2: Handler does not return null
        if (handleHelloRequest() != null) {
            System.out.println("Test 2 Passed: Handler does not return null.");
        } else {
            System.out.println("Test 2 Failed.");
        }

        // Test 3: Handler returns a non-empty string
        if (!handleHelloRequest().isEmpty()) {
            System.out.println("Test 3 Passed: Handler returns a non-empty string.");
        } else {
            System.out.println("Test 3 Failed.");
        }

        // Test 4: Check string length
        if (handleHelloRequest().length() == 13) {
            System.out.println("Test 4 Passed: Handler returns string of correct length.");
        } else {
            System.out.println("Test 4 Failed.");
        }

        // Test 5: Check for specific content
        if (handleHelloRequest().contains("World")) {
            System.out.println("Test 5 Passed: Handler string contains 'World'.");
        } else {
            System.out.println("Test 5 Failed.");
        }
        System.out.println("--- Tests Finished ---\n");
    }

    public static void main(String[] args) {
        runTests();

        // The web application part follows the prompt's description.
        // Line 1: (import static spark.Spark.*) - imports the web application class/methods.
        
        // Line 2: The application object is managed implicitly by the Spark framework.
        port(5001); // Set a port for the application

        // Line 3: A decorator equivalent that tells which URL calls the function.
        // Line 4: The function to be called (as a lambda expression).
        // Line 5: The message returned to the user.
        get("/", (req, res) -> {
            // Setting a specific content type is a good security practice
            // to prevent browsers from misinterpreting the content.
            res.type("text/plain; charset=utf-8");
            return handleHelloRequest();
        });

        // Line 6: Runs the application (server starts implicitly with route mapping).
        System.out.println("Server is running on http://localhost:5001");
        System.out.println("Press Ctrl+C to stop.");
        
        // Block the main thread to keep the server running
        awaitInitialization();
    }
}