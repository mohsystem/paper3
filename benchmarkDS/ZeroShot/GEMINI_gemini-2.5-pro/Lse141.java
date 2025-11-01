import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.Optional;

public class Lse141 {

    /**
     * Securely retrieves the database server name from an environment variable.
     * Falls back to a default value if the environment variable is not set.
     * @return The database server name.
     */
    public static String getServerName() {
        // Best practice: Store configuration in environment variables, not in code.
        return Optional.ofNullable(System.getenv("DB_SERVER")).orElse("localhost");
    }

    /**
     * Securely retrieves the database name from an environment variable.
     * Falls back to a default value if the environment variable is not set.
     * @return The database name.
     */
    public static String getDatabaseName() {
        return Optional.ofNullable(System.getenv("DB_NAME")).orElse("mydatabase");
    }

    /**
     * Securely retrieves the database user from an environment variable.
     * Falls back to a default value if the environment variable is not set.
     * @return The database user.
     */
    public static String getDatabaseUser() {
        return Optional.ofNullable(System.getenv("DB_USER")).orElse("admin");
    }

    /**
     * Securely retrieves the database password from an environment variable.
     * WARNING: Avoid hardcoding passwords. This fallback is for demonstration only.
     * @return The database password.
     */
    public static String getDatabasePassword() {
        // In a real application, throw an error if the password is not set in the environment.
        return Optional.ofNullable(System.getenv("DB_PASSWORD")).orElse("INSECURE_DEFAULT_PASSWORD");
    }

    /**
     * Constructs a database connection string from configuration values.
     * Note: This does not establish a real connection, it only builds the string.
     * @return A formatted JDBC-like connection string.
     */
    public static String getDatabaseConnection() {
        return String.format("jdbc:mysql://%s/%s?user=%s&password=%s",
                getServerName(),
                getDatabaseName(),
                getDatabaseUser(),
                "[REDACTED]"); // Avoid exposing password in connection strings that might be logged.
    }

    static class ConfigHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            StringBuilder response = new StringBuilder();
            response.append("<html><body>");
            response.append("<h1>Database Configuration</h1>");
            response.append("<ul>");
            response.append("<li><b>Server Name:</b> ").append(getServerName()).append("</li>");
            response.append("<li><b>Database Name:</b> ").append(getDatabaseName()).append("</li>");
            response.append("<li><b>User:</b> ").append(getDatabaseUser()).append("</li>");
            response.append("<li><b>Password:</b> ").append("[REDACTED FOR SECURITY]").append("</li>");
            response.append("<li><b>Connection String:</b> ").append(getDatabaseConnection()).append("</li>");
            response.append("</ul>");
            response.append("</body></html>");

            String responseString = response.toString();
            t.sendResponseHeaders(200, responseString.getBytes().length);
            OutputStream os = t.getResponseBody();
            os.write(responseString.getBytes());
            os.close();
        }
    }
    
    public static void main(String[] args) throws IOException {
        // 1. Run 5 Test Cases
        System.out.println("--- Running Test Cases ---");
        System.out.println("1. Server Name: " + getServerName());
        System.out.println("2. Database Name: " + getDatabaseName());
        System.out.println("3. Database User: " + getDatabaseUser());
        System.out.println("4. Database Password: " + getDatabasePassword());
        System.out.println("5. Full Connection String: " + getDatabaseConnection());
        System.out.println("--------------------------\n");
        System.out.println("To set these values, use environment variables like:");
        System.out.println("export DB_SERVER=db.example.com");
        System.out.println("export DB_NAME=prod_db");
        System.out.println("export DB_USER=myuser");
        System.out.println("export DB_PASSWORD='my_secret_password'\n");

        // 2. Create and start the web app
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/config", new ConfigHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Visit http://localhost:8080/config to see the configuration.");
    }
}