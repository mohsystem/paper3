import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Optional;

import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

/**
 * Main class for the web application.
 * To run this example, you need to set the following environment variables:
 * export DB_HOST="your_db_host"
 * export DB_NAME="your_db_name"
 * export DB_USER="your_db_user"
 * export DB_PASSWORD="your_db_password"
 */
public class Lse141 {

    /**
     * Creates and starts a simple web server.
     *
     * @return The created HttpServer instance.
     * @throws IOException if an I/O error occurs.
     */
    public static HttpServer createWebApp() throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new MyHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port 8080");
        return server;
    }

    /**
     * Retrieves the server name from an environment variable.
     *
     * @return The database server name.
     */
    public static String getServerName() {
        return System.getenv("DB_HOST");
    }

    /**
     * Retrieves the database name from an environment variable.
     *
     * @return The database name.
     */
    public static String getDbName() {
        return System.getenv("DB_NAME");
    }

    /**
     * Retrieves the database user from an environment variable.
     *
     * @return The database user.
     */
    public static String getDbUser() {
        return System.getenv("DB_USER");
    }

    /**
     * Retrieves the database password from an environment variable.
     *
     * @return The database password.
     */
    public static String getDbPassword() {
        return System.getenv("DB_PASSWORD");
    }

    /**
     * Establishes a database connection using credentials from environment variables.
     * This is a demonstration and will not connect without a proper JDBC driver and running database.
     *
     * @return A database Connection object, or null if credentials are not set or connection fails.
     */
    public static Connection getDbConnection() {
        String serverName = getServerName();
        String dbName = getDbName();
        String user = getDbUser();
        String password = getDbPassword();

        if (serverName == null || dbName == null || user == null || password == null) {
            System.err.println("Database environment variables are not fully set.");
            return null;
        }

        // Example for PostgreSQL JDBC URL
        String url = "jdbc:postgresql://" + serverName + "/" + dbName;
        Connection conn = null;
        try {
            // In a real application, ensure the JDBC driver is on the classpath.
            // conn = DriverManager.getConnection(url, user, password);
            System.out.println("Demonstration: Would attempt to connect to: " + url);
        } catch (Exception e) { // Catches SQLException and others like ClassNotFoundException
            System.err.println("Failed to create a database connection (demonstration).");
            // In a real app, log the error properly: e.g., log.error("Connection failed", e);
        }
        return conn;
    }

    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String response = "Web App is running.";
            t.sendResponseHeaders(200, response.length());
            try (OutputStream os = t.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }

    public static void main(String[] args) {
        // Start the web app
        try {
            createWebApp();
        } catch (IOException e) {
            System.err.println("Could not start web server: " + e.getMessage());
        }

        // --- 5 Test Cases ---
        System.out.println("\n--- Running 5 Test Cases ---");

        // Test Case 1: Get Server Name
        System.out.println("1. Server Name: " + Optional.ofNullable(getServerName()).orElse("Not Set"));

        // Test Case 2: Get DB Name
        System.out.println("2. Database Name: " + Optional.ofNullable(getDbName()).orElse("Not Set"));

        // Test Case 3: Get DB User
        System.out.println("3. Database User: " + Optional.ofNullable(getDbUser()).orElse("Not Set"));

        // Test Case 4: Check for DB Password
        String password = getDbPassword();
        System.out.println("4. Database Password: " + (password != null && !password.isEmpty() ? "Set" : "Not Set"));

        // Test Case 5: Attempt to get a DB connection object
        System.out.println("5. Attempting to get DB Connection object...");
        getDbConnection();
        System.out.println("--- Test Cases Finished ---");

    }
}