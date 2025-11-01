import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.sql.Connection;
import java.sql.DriverManager;
import java.util.Objects;
import java.util.Properties;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Lse141 {

    // Create a minimal web app using JDK's built-in HttpServer
    public static HttpServer createWebApp(int port) throws IOException {
        InetSocketAddress bindAddr = new InetSocketAddress(InetAddress.getByName("127.0.0.1"), port);
        HttpServer server = HttpServer.create(bindAddr, 50);
        server.createContext("/", new RootHandler());
        server.createContext("/health", new HealthHandler());
        ExecutorService executor = Executors.newFixedThreadPool(2, r -> {
            Thread t = new Thread(r, "webapp-worker");
            t.setDaemon(true);
            return t;
        });
        server.setExecutor(executor);
        server.start();
        return server;
    }

    static final class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            byte[] resp = "{\"status\":\"ok\"}".getBytes(StandardCharsets.UTF_8);
            Headers headers = exchange.getResponseHeaders();
            headers.add("Content-Type", "application/json; charset=utf-8");
            exchange.sendResponseHeaders(200, resp.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(resp);
            }
        }
    }

    static final class HealthHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            byte[] resp = "healthy".getBytes(StandardCharsets.UTF_8);
            Headers headers = exchange.getResponseHeaders();
            headers.add("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(200, resp.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(resp);
            }
        }
    }

    // Securely obtain DB settings from environment (no hardcoded secrets)
    public static String getServerName() {
        String host = System.getenv("DB_HOST");
        return host != null && !host.isEmpty() ? host : "127.0.0.1";
    }

    public static String getDatabaseName() {
        String name = System.getenv("DB_NAME");
        return name != null && !name.isEmpty() ? name : "appdb";
    }

    public static String getDatabaseUser() {
        String user = System.getenv("DB_USER");
        return user != null ? user : "";
    }

    // Do not log or print this value
    public static String getDatabasePassword() {
        String pwd = System.getenv("DB_PASSWORD");
        return pwd != null ? pwd : "";
    }

    // Returns a JDBC Connection if explicitly enabled; otherwise returns null by default.
    // Avoids hardcoding credentials and avoids attempting network connections unintentionally.
    public static Connection getDatabaseConnection() {
        String connectFlag = System.getenv("DB_CONNECT");
        if (!"true".equalsIgnoreCase(connectFlag)) {
            return null;
        }

        String host = getServerName();
        String db = getDatabaseName();
        String user = getDatabaseUser();
        String pwd = getDatabasePassword();
        String port = System.getenv("DB_PORT");
        if (port == null || port.isEmpty()) {
            port = "5432";
        }
        String sslmode = System.getenv("DB_SSLMODE");
        if (sslmode == null || sslmode.isEmpty()) {
            sslmode = "verify-full";
        }

        // Example PostgreSQL JDBC URL; driver must be on classpath to actually connect.
        // If driver/env not present, we return null instead of throwing.
        String url = String.format("jdbc:postgresql://%s:%s/%s?sslmode=%s", host, port, db, sslmode);
        Properties props = new Properties();
        props.setProperty("user", user != null ? user : "");
        props.setProperty("password", pwd != null ? pwd : "");
        // Optional sane timeouts if the driver supports them
        props.setProperty("loginTimeout", "5");

        try {
            // Will return a live connection only if driver and env are correct.
            return DriverManager.getConnection(url, props);
        } catch (Exception e) {
            // Fail closed without leaking details or secrets
            return null;
        }
    }

    // 5 simple test cases in main
    public static void main(String[] args) throws Exception {
        // Start web app on ephemeral port (0 lets the OS choose)
        HttpServer server = createWebApp(0);
        int actualPort = server.getAddress().getPort();
        System.out.println("[Test1] Web app started on 127.0.0.1:" + actualPort);

        // Test2: Server name (DB host)
        String serverName = getServerName();
        System.out.println("[Test2] DB Server Name: " + serverName);

        // Test3: Database name
        String dbName = getDatabaseName();
        System.out.println("[Test3] DB Name: " + dbName);

        // Test4: Database user
        String dbUser = getDatabaseUser();
        System.out.println("[Test4] DB User: " + (dbUser.isEmpty() ? "(not set)" : dbUser));

        // Test5: Password presence (do not print the secret)
        String pwd = getDatabasePassword();
        System.out.println("[Test5] DB Password set: " + (!Objects.toString(pwd, "").isEmpty()));

        // Optional: Attempt connection if DB_CONNECT=true (likely returns null in most environments)
        Connection conn = getDatabaseConnection();
        if (conn != null) {
            try {
                System.out.println("[Extra] DB Connection acquired: " + !conn.isClosed());
            } finally {
                try {
                    conn.close();
                } catch (Exception ignored) {
                }
            }
        } else {
            System.out.println("[Extra] DB Connection not acquired (expected unless DB_CONNECT=true and driver present).");
        }

        // Stop server
        server.stop(0);
        System.out.println("[Done] Web app stopped.");
    }
}