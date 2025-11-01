/*
To compile and run this Java code:
1. You need Apache Maven.
2. Create a pom.xml file with the following content:
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
    <modelVersion>4.0.0</modelVersion>
    <groupId>com.example</groupId>
    <artifactId>spark-app</artifactId>
    <version>1.0-SNAPSHOT</version>
    <properties>
        <maven.compiler.source>1.8</maven.compiler.source>
        <maven.compiler.target>1.8</maven.compiler.target>
    </properties>
    <dependencies>
        <dependency>
            <groupId>com.sparkjava</groupId>
            <artifactId>spark-core</artifactId>
            <version>2.9.4</version>
        </dependency>
        <dependency>
            <groupId>com.google.code.gson</groupId>
            <artifactId>gson</artifactId>
            <version>2.10.1</version>
        </dependency>
    </dependencies>
</project>
3. Save the Java code as Lse42.java in src/main/java/Lse42.java.
4. Run from the project root directory:
   mvn compile exec:java -Dexec.mainClass="Lse42"
*/
import static spark.Spark.*;

import java.util.HashMap;
import java.util.Map;
import com.google.gson.Gson;


public class Lse42 {

    public static void main(String[] args) {
        // Use a port other than the default 4567
        port(8080);
        
        // In production, you must use TLS (HTTPS).
        // For SparkJava, this would involve a secureKeystore() call.

        // This is a simplified test route to simulate a login and establish a session.
        // In a real application, this would be a POST request with credential validation.
        get("/login_test/:username", (req, res) -> {
            // NOTE: Storing sensitive data like SSN in a session should be done with extreme caution.
            // The session data is stored on the server, but the session ID is sent to the client.
            // Always ensure transport is secure (HTTPS).
            req.session(true); // Create session if it doesn't exist
            req.session().attribute("username", req.params(":username"));
            req.session().attribute("email", req.params(":username") + "@example.com");
            // SECURITY: Storing raw PII like SSN is highly discouraged.
            req.session().attribute("ssn", "000-00-0000"); // DUMMY SSN for demonstration only
            return "Session created for user: " + req.params(":username");
        });

        // The main /info route as requested by the prompt.
        get("/info", (req, res) -> {
            // Ensure a session exists and contains the required data before proceeding.
            if (req.session(false) == null || req.session().attribute("username") == null) {
                res.status(401); // Unauthorized
                return "{\"error\": \"Unauthorized: Please log in first.\"}";
            }

            String username = req.session().attribute("username");
            String email = req.session().attribute("email");
            String ssn = req.session().attribute("ssn");

            // SECURITY WARNING: Exposing sensitive information like an SSN in an API response
            // is extremely dangerous and should be avoided. This is done here only to fulfill
            // the prompt's requirements. In a real-world application, such data should
            // be masked or not exposed at all.
            Map<String, String> userInfo = new HashMap<>();
            userInfo.put("username", username);
            userInfo.put("email", email);
            userInfo.put("ssn", ssn);
            
            res.type("application/json");
            return new Gson().toJson(userInfo);
        });

        // A route to invalidate the session for testing purposes.
        get("/logout_test", (req, res) -> {
            if (req.session(false) != null) {
                req.session().invalidate();
            }
            return "Session invalidated.";
        });
        
        System.out.println("Server started on http://localhost:8080");
        System.out.println("NOTE: For production, use a reverse proxy to handle HTTPS.");
        System.out.println("\n--- Test Cases (run in a separate terminal) ---");
        System.out.println("1. Access /info without session (should fail): curl http://localhost:8080/info");
        System.out.println("2. Log in as 'alice' to create session: curl -c cookie.txt http://localhost:8080/login_test/alice");
        System.out.println("3. Access /info with session (should succeed): curl -b cookie.txt http://localhost:8080/info");
        System.out.println("4. Log out to destroy session: curl -b cookie.txt http://localhost:8080/logout_test");
        System.out.println("5. Access /info after logout (should fail): curl -b cookie.txt http://localhost:8080/info");
    }
}