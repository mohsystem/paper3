import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Consumer;

public class Lse113 {

    // Simulates a routing table, mapping a URL path to a function.
    private static final Map<String, Consumer<String>> routes = new HashMap<>();

    /**
     * The handler function that performs the ping operation.
     * @param url The domain or IP address to ping.
     */
    public static void ping(String url) {
        try {
            String command;
            // Differentiate command for Windows and other OS
            if (System.getProperty("os.name").toLowerCase().startsWith("windows")) {
                // -n 1 sends 1 echo request on Windows
                command = "ping -n 1 " + url;
            } else {
                // -c 1 sends 1 packet on Unix-like systems
                command = "ping -c 1 " + url;
            }

            System.out.println("Executing: " + command);
            Process process = Runtime.getRuntime().exec(command);

            // Wait for the command to complete and get the exit code
            int exitCode = process.waitFor();
            String pong = "URL " + url;

            if (exitCode == 0) {
                System.out.println(pong + " is alive.");
            } else {
                System.out.println(pong + " is not alive.");
            }
        } catch (IOException | InterruptedException e) {
            System.err.println("Error executing ping command for " + url + ": " + e.getMessage());
        }
    }

    /**
     * Simulates a decorator that binds a function to a URL path.
     * @param path The URL path (e.g., "/ping").
     * @param handler The function to execute for this path.
     */
    public static void route(String path, Consumer<String> handler) {
        routes.put(path, handler);
    }

    /**
     * Simulates handling a web request. It looks up the path and executes the associated function.
     * @param path The requested URL path.
     * @param url The parameter (target URL) for the handler function.
     */
    public static void handleRequest(String path, String url) {
        Consumer<String> handler = routes.get(path);
        if (handler != null) {
            handler.accept(url);
        } else {
            System.out.println("No handler found for path: " + path);
        }
    }

    public static void main(String[] args) {
        // "Decorate" the ping function by binding it to the "/ping" URL path.
        route("/ping", Lse113::ping);

        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: A known alive host
        handleRequest("/ping", "google.com");
        System.out.println();

        // Test Case 2: Localhost, should be alive
        handleRequest("/ping", "127.0.0.1");
        System.out.println();

        // Test Case 3: A non-existent domain, should not be alive
        handleRequest("/ping", "nonexistentdomain12345.com");
        System.out.println();
        
        // Test Case 4: An unroutable IP address
        handleRequest("/ping", "192.0.2.0"); // Reserved for documentation, should fail
        System.out.println();

        // Test Case 5: A request to a URL path that is not registered
        handleRequest("/status", "google.com");
        System.out.println();
    }
}