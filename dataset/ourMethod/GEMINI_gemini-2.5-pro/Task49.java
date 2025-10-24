/*
 * To compile and run this code, you need the following dependencies:
 * 1. SQLite JDBC Driver: org.xerial:sqlite-jdbc
 * 2. JSON in Java: org.json:json
 *
 * If you are using Maven, add these to your pom.xml:
 * <dependencies>
 *     <dependency>
 *         <groupId>org.xerial</groupId>
 *         <artifactId>sqlite-jdbc</artifactId>
 *         <version>3.45.1.0</version>
 *     </dependency>
 *     <dependency>
 *         <groupId>org.json</groupId>
 *         <artifactId>json</artifactId>
 *         <version>20231013</version>
 *     </dependency>
 * </dependencies>
 *
 * Then compile and run:
 * mvn exec:java -Dexec.mainClass="Task49"
 *
 * Or without Maven, download the jars and compile/run from the command line:
 * javac -cp "sqlite-jdbc-3.45.1.0.jar:json-20231013.jar:." Task49.java
 * java -cp "sqlite-jdbc-3.45.1.0.jar:json-20231013.jar:." Task49
 */
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.json.JSONException;
import org.json.JSONObject;

import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.sql.*;
import java.util.Base64;
import java.util.concurrent.Executors;

public class Task49 {

    private static final String DATABASE_URL = "jdbc:sqlite:users.db";
    private static final int PORT = 8080;

    public static void main(String[] args) throws IOException {
        setupDatabase();

        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/api/users", new UserHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();

        System.out.println("Server started on port " + PORT);
        System.out.println("Use Ctrl+C to stop the server.");
        printTestCases();
    }

    private static void setupDatabase() {
        String sql = "CREATE TABLE IF NOT EXISTS users (\n"
                + " id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                + " username TEXT NOT NULL UNIQUE,\n"
                + " email TEXT NOT NULL,\n"
                + " password_hash TEXT NOT NULL,\n"
                + " salt TEXT NOT NULL\n"
                + ");";

        try (Connection conn = DriverManager.getConnection(DATABASE_URL);
             Statement stmt = conn.createStatement()) {
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database setup failed: " + e.getMessage());
            System.exit(1);
        }
    }

    static class UserHandler implements HttpHandler {

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    if ("/api/users".equals(exchange.getRequestURI().getPath())) {
                        handleCreateUser(exchange);
                    } else {
                        sendResponse(exchange, 404, "{\"error\":\"Not Found\"}");
                    }
                } else {
                    sendResponse(exchange, 405, "{\"error\":\"Method Not Allowed\"}");
                }
            } catch (Exception e) {
                e.printStackTrace();
                sendResponse(exchange, 500, "{\"error\":\"Internal Server Error\"}");
            }
        }

        private void handleCreateUser(HttpExchange exchange) throws IOException {
            String body = readRequestBody(exchange);
            if (body.isEmpty()) {
                sendResponse(exchange, 400, "{\"error\":\"Request body is empty\"}");
                return;
            }

            try {
                JSONObject userData = new JSONObject(body);

                if (!userData.has("username") || !userData.has("email") || !userData.has("password")) {
                    sendResponse(exchange, 400, "{\"error\":\"Missing fields: username, email, and password are required\"}");
                    return;
                }
                
                String username = userData.getString("username");
                String email = userData.getString("email");
                String password = userData.getString("password");

                if (username.trim().isEmpty() || email.trim().isEmpty() || password.isEmpty()) {
                     sendResponse(exchange, 400, "{\"error\":\"Username, email, and password cannot be empty\"}");
                    return;
                }

                byte[] salt = generateSalt();
                String passwordHash = hashPassword(password, salt);
                String saltString = Base64.getEncoder().encodeToString(salt);

                String sql = "INSERT INTO users(username, email, password_hash, salt) VALUES(?,?,?,?)";

                try (Connection conn = DriverManager.getConnection(DATABASE_URL);
                     PreparedStatement pstmt = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS)) {
                    pstmt.setString(1, username);
                    pstmt.setString(2, email);
                    pstmt.setString(3, passwordHash);
                    pstmt.setString(4, saltString);
                    pstmt.executeUpdate();

                    try (ResultSet generatedKeys = pstmt.getGeneratedKeys()) {
                        if (generatedKeys.next()) {
                            long id = generatedKeys.getLong(1);
                            String response = String.format("{\"message\":\"User created successfully\", \"id\":%d}", id);
                            sendResponse(exchange, 201, response);
                        } else {
                             sendResponse(exchange, 500, "{\"error\":\"Failed to create user, no ID obtained.\"}");
                        }
                    }
                } catch (SQLException e) {
                    if (e.getErrorCode() == 19 && e.getMessage().contains("UNIQUE constraint failed")) { // SQLite error code for UNIQUE constraint
                        sendResponse(exchange, 409, "{\"error\":\"Username already exists\"}");
                    } else {
                        e.printStackTrace();
                        sendResponse(exchange, 500, "{\"error\":\"Database error\"}");
                    }
                }

            } catch (JSONException e) {
                sendResponse(exchange, 400, "{\"error\":\"Invalid JSON format\"}");
            } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
                e.printStackTrace();
                sendResponse(exchange, 500, "{\"error\":\"Internal security configuration error\"}");
            }
        }

        private String readRequestBody(HttpExchange exchange) throws IOException {
            try (InputStream is = exchange.getRequestBody()) {
                return new String(is.readAllBytes(), StandardCharsets.UTF_8);
            }
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "application/json; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }

        private byte[] generateSalt() {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);
            return salt;
        }

        private String hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
            int iterations = 600000;
            int keyLength = 256;
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, iterations, keyLength);
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            return Base64.getEncoder().encodeToString(hash);
        }
    }

    private static void printTestCases() {
        String baseUrl = "http://localhost:" + PORT + "/api/users";
        System.out.println("\n--- API Test Cases ---");
        System.out.println("Run these curl commands in a separate terminal.\n");

        // Test Case 1: Successful user creation
        System.out.println("1. Create a new user (Success):");
        System.out.println("curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"testuser1\",\"email\":\"test1@example.com\",\"password\":\"SecurePass1!\"}' " + baseUrl);
        System.out.println();

        // Test Case 2: Attempt to create a user with a duplicate username
        System.out.println("2. Create a duplicate user (Conflict):");
        System.out.println("curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"testuser1\",\"email\":\"another@example.com\",\"password\":\"AnotherPass1!\"}' " + baseUrl);
        System.out.println();

        // Test Case 3: Request with a missing field
        System.out.println("3. Request with missing 'password' field (Bad Request):");
        System.out.println("curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"testuser2\",\"email\":\"test2@example.com\"}' " + baseUrl);
        System.out.println();

        // Test Case 4: Request with malformed JSON
        System.out.println("4. Request with malformed JSON (Bad Request):");
        System.out.println("curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"testuser3\", \"email\":\"test3@example.com\",' " + baseUrl);
        System.out.println();

        // Test Case 5: Request to a wrong endpoint
        System.out.println("5. Request to a non-existent endpoint (Not Found):");
        System.out.println("curl -X POST -H \"Content-Type: application/json\" -d '{\"data\":\"some data\"}' http://localhost:" + PORT + "/api/nonexistent");
        System.out.println();
    }
}