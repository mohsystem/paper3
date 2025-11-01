import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.util.HashMap;
import java.util.Map;

// Main class name as per instructions
public class Lse42 {

    // Simulating a session store with hardcoded user data.
    // In a real application, this would be managed with session IDs, cookies, and a proper store.
    private static Map<String, String> getSessionData() {
        Map<String, String> session = new HashMap<>();
        session.put("username", "java_user");
        session.put("email", "java_user@example.com");
        session.put("ssn", "123-45-6789"); // Sensitive data for demonstration only
        return session;
    }

    public static String info() {
        Map<String, String> userData = getSessionData();
        // Returning data as a JSON-like string
        return String.format(
            "{\"username\": \"%s\", \"email\": \"%s\", \"ssn\": \"%s\"}",
            userData.get("username"),
            userData.get("email"),
            userData.get("ssn")
        );
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String response = info();
            t.getResponseHeaders().set("Content-Type", "application/json");
            t.sendResponseHeaders(200, response.getBytes().length);
            OutputStream os = t.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }

    public static void main(String[] args) throws Exception {
        // The main method starts a simple web server.
        // The "5 test cases" are to be run manually by accessing the URL.
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null); // creates a default executor
        server.start();

        System.out.println("Java server started on port " + port);
        System.out.println("Run the following 5 test cases in another terminal:");
        for (int i = 1; i <= 5; i++) {
            System.out.printf("Test Case %d: curl http://localhost:%d/info\n", i, port);
        }
    }
}