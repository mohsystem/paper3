import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.concurrent.Executors;

public class Lse141 {

    public static class DbConfig {
        public final String server;
        public final String database;
        public final String user;
        public final String password;

        public DbConfig(String server, String database, String user, String password) {
            this.server = server;
            this.database = database;
            this.user = user;
            this.password = password;
        }
    }

    public static class DbConnection {
        public final String server;
        public final String database;
        public final String user;
        public final boolean connected;

        public DbConnection(String server, String database, String user, boolean connected) {
            this.server = server;
            this.database = database;
            this.user = user;
            this.connected = connected;
        }

        @Override
        public String toString() {
            return "DbConnection{server='" + server + "', database='" + database + "', user='" + user + "', connected=" + connected + "}";
        }
    }

    // Create a web app (simple HTTP server)
    public static HttpServer createWebApp(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String response = "Hello from Java web app";
                exchange.sendResponseHeaders(200, response.getBytes().length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes());
                }
            }
        });
        server.setExecutor(Executors.newSingleThreadExecutor());
        return server;
    }

    public static String getServerName(DbConfig config) {
        return config.server;
    }

    public static String getDatabaseName(DbConfig config) {
        return config.database;
    }

    public static String getDatabaseUser(DbConfig config) {
        return config.user;
    }

    public static String getDatabasePassword(DbConfig config) {
        return config.password;
    }

    public static DbConnection getDatabaseConnection(DbConfig config) {
        // Mock connection object
        return new DbConnection(config.server, config.database, config.user, true);
    }

    public static void main(String[] args) throws Exception {
        DbConfig config = new DbConfig("localhost", "sampledb", "admin", "secret");

        // Test case 1: Server name
        System.out.println("Server Name: " + getServerName(config));

        // Test case 2: Database name
        System.out.println("Database Name: " + getDatabaseName(config));

        // Test case 3: Database user
        System.out.println("Database User: " + getDatabaseUser(config));

        // Test case 4: Database password
        System.out.println("Database Password: " + getDatabasePassword(config));

        // Test case 5: Database connection
        DbConnection conn = getDatabaseConnection(config);
        System.out.println("Database Connection: " + conn);

        // Start web app
        HttpServer webApp = createWebApp(9090);
        webApp.start();
        System.out.println("Java web app started on http://127.0.0.1:9090/");
        // Stop immediately for demonstration
        webApp.stop(0);
        System.out.println("Java web app stopped.");
    }
}