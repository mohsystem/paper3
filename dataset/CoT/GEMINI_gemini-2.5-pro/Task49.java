/*
NOTE: To compile and run this code, you need the SQLite JDBC driver.
1. Download the driver JAR from: https://github.com/xerial/sqlite-jdbc/releases
2. Compile: javac Task49.java
3. Run: java -cp ".:sqlite-jdbc-3.xx.x.jar" Task49
   (Replace 'xx.x' with the downloaded version and use ';' instead of ':' on Windows)
*/
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Task49 {

    private static final String DB_URL = "jdbc:sqlite:users_java.db";

    // 1. Initialize Database
    private static void initDb() {
        // Use try-with-resources to ensure the connection and statement are closed
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users ("
                       + " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                       + " name TEXT NOT NULL,"
                       + " email TEXT NOT NULL UNIQUE"
                       + ");";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization failed: " + e.getMessage());
            // In a real application, you might want to exit if the DB can't be set up.
        }
    }

    // 2. Core Logic to add a user with validation and security
    public static boolean addUser(String name, String email) {
        // --- Input Validation ---
        // Check for null, empty, or excessively long inputs
        if (name == null || name.trim().isEmpty() || name.length() > 100) {
            System.err.println("Validation failed: Invalid name provided.");
            return false;
        }
        // Use a standard regex for basic email format validation
        final Pattern emailPattern = Pattern.compile("^[a-zA-Z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-zA-Z0-9.-]+$");
        if (email == null || email.trim().isEmpty() || email.length() > 100 || !emailPattern.matcher(email).matches()) {
             System.err.println("Validation failed: Invalid email provided.");
            return false;
        }

        String sql = "INSERT INTO users(name, email) VALUES(?, ?)";
        
        // --- Secure Database Operation ---
        // Use try-with-resources for automatic resource management
        // Use PreparedStatement to prevent SQL Injection vulnerabilities
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, name);
            pstmt.setString(2, email);
            pstmt.executeUpdate();
            return true;
        } catch (SQLException e) {
            // Handle specific errors like unique constraint violation for better feedback
            if (e.getErrorCode() == 19 && e.getMessage().contains("UNIQUE constraint failed")) {
                System.err.println("Database error: Email '" + email + "' already exists.");
            } else {
                 System.err.println("Database error on insert: " + e.getMessage());
            }
            return false;
        }
    }

    // 3. HTTP Handler for the API endpoint
    static class UserHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "";
            int statusCode = 200;

            try {
                if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
                    if (contentType == null || !contentType.toLowerCase().contains("application/json")) {
                        statusCode = 415; // Unsupported Media Type
                        response = "{\"error\":\"Content-Type must be application/json\"}";
                    } else {
                        InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
                        String body = new BufferedReader(isr).lines().collect(Collectors.joining("\n"));
                        
                        // NOTE: Manual JSON parsing is fragile and not recommended for production.
                        // Use a robust library like Gson, Jackson, or org.json instead.
                        String name = parseJsonField(body, "name");
                        String email = parseJsonField(body, "email");

                        if (name == null || email == null) {
                             statusCode = 400; // Bad Request
                             response = "{\"error\":\"'name' and 'email' fields are required and must be strings\"}";
                        } else {
                            if (addUser(name, email)) {
                                statusCode = 201; // Created
                                response = "{\"message\":\"User created successfully\"}";
                            } else {
                                // The reason for failure (validation or duplicate) is logged in addUser.
                                // The API returns a generic but appropriate error.
                                statusCode = 409; // Conflict (common for duplicates) or 400 for bad data
                                response = "{\"error\":\"Failed to create user. Invalid data or email already exists.\"}";
                            }
                        }
                    }
                } else {
                    statusCode = 405; // Method Not Allowed
                    response = "{\"error\":\"Only POST method is allowed\"}";
                }
            } catch (Exception e) {
                statusCode = 500; // Internal Server Error
                response = "{\"error\":\"An internal server error occurred\"}";
                System.err.println("Server error: " + e.getMessage());
            } finally {
                exchange.getResponseHeaders().set("Content-Type", "application/json; charset=UTF-8");
                exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            }
        }

        // A very basic and non-robust JSON parser for demonstration purposes.
        private String parseJsonField(String json, String field) {
            String pattern = "\"" + field + "\"\\s*:\\s*\"(.*?)\"";
            java.util.regex.Matcher matcher = Pattern.compile(pattern).matcher(json);
            if (matcher.find()) {
                // Basic un-escaping for common characters
                return matcher.group(1).replace("\\\"", "\"").replace("\\\\", "\\");
            }
            return null;
        }
    }
    
    // 4. Main method with test cases and server startup
    public static void main(String[] args) throws IOException {
        initDb();
        
        System.out.println("--- Running Logical Test Cases ---");
        // Test Case 1: Valid user
        System.out.println("\nTest 1: Add valid user 'Alice'");
        boolean success1 = addUser("Alice", "alice.test@example.com");
        System.out.println("Result: " + (success1 ? "SUCCESS" : "FAILURE"));

        // Test Case 2: Duplicate email
        System.out.println("\nTest 2: Add user with duplicate email");
        boolean success2 = addUser("Alice Smith", "alice.test@example.com");
        System.out.println("Result: " + (!success2 ? "SUCCESS (Correctly failed)" : "FAILURE (Should have failed)"));
        
        // Test Case 3: Invalid email format
        System.out.println("\nTest 3: Add user with invalid email");
        boolean success3 = addUser("Bob", "bob-invalid-email");
        System.out.println("Result: " + (!success3 ? "SUCCESS (Correctly failed)" : "FAILURE (Should have failed)"));
        
        // Test Case 4: Null name
        System.out.println("\nTest 4: Add user with null name");
        boolean success4 = addUser(null, "charlie@example.com");
        System.out.println("Result: " + (!success4 ? "SUCCESS (Correctly failed)" : "FAILURE (Should have failed)"));
        
        // Test Case 5: Name too long
        System.out.println("\nTest 5: Add user with name > 100 chars");
        String longName = new String(new char[101]).replace('\0', 'a');
        boolean success5 = addUser(longName, "dave@example.com");
        System.out.println("Result: " + (!success5 ? "SUCCESS (Correctly failed)" : "FAILURE (Should have failed)"));
        System.out.println("\n--- Test Cases Finished ---\n");

        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/api/users", new UserHandler());
        server.setExecutor(null); // Use the default executor
        server.start();
        System.out.println("Server started on port 8080.");
        System.out.println("Test API with: curl -X POST -H \"Content-Type: application/json\" -d '{\"name\":\"Eve\", \"email\":\"eve@example.com\"}' http://localhost:8080/api/users");
    }
}