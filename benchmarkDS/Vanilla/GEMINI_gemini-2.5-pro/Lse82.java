import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Lse82 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api", new ApiHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Use Ctrl+C to stop the server.");
        System.out.println("\n--- Test Cases ---");
        System.out.println("Run the following commands in your terminal:");
        System.out.println("1) Test Case 1 (Standard):");
        System.out.println("   curl \"http://localhost:8080/api?payload=%7B%22name%22%3A%22JohnDoe%22%2C%22age%22%3A30%7D\"");
        System.out.println("   Expected output: JohnDoe\n");
        System.out.println("2) Test Case 2 (Different Name):");
        System.out.println("   curl \"http://localhost:8080/api?payload=%7B%22name%22%3A%22JaneSmith%22%7D\"");
        System.out.println("   Expected output: JaneSmith\n");
        System.out.println("3) Test Case 3 (No payload parameter):");
        System.out.println("   curl \"http://localhost:8080/api\"");
        System.out.println("   Expected output: Payload parameter is missing.\n");
        System.out.println("4) Test Case 4 (Payload without name key):");
        System.out.println("   curl \"http://localhost:8080/api?payload=%7B%22city%22%3A%22NewYork%22%7D\"");
        System.out.println("   Expected output: 'name' key not found in payload.\n");
        System.out.println("5) Test Case 5 (Malformed payload):");
        System.out.println("   curl \"http://localhost:8080/api?payload=thisisnotjson\"");
        System.out.println("   Expected output: 'name' key not found in payload.\n");
    }

    static class ApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            Map<String, String> params = queryToMap(query);
            String payload = params.get("payload");
            
            String response;
            int statusCode = 200;

            if (payload == null) {
                response = "Payload parameter is missing.";
                statusCode = 400;
            } else {
                // In a real app, use a proper JSON library (e.g., Gson, Jackson).
                // For this example, we use a simple regex to avoid external dependencies.
                Map<String, String> dict = parseSimpleJson(payload);
                String name = dict.get("name");
                if (name != null) {
                    response = name;
                } else {
                    response = "'name' key not found in payload.";
                    statusCode = 400;
                }
            }
            
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes(StandardCharsets.UTF_8));
            os.close();
        }
    }

    public static Map<String, String> queryToMap(String query) {
        Map<String, String> result = new HashMap<>();
        if (query == null) {
            return result;
        }
        for (String param : query.split("&")) {
            String[] entry = param.split("=");
            if (entry.length > 1) {
                try {
                    String key = URLDecoder.decode(entry[0], StandardCharsets.UTF_8.name());
                    String value = URLDecoder.decode(entry[1], StandardCharsets.UTF_8.name());
                    result.put(key, value);
                } catch (java.io.UnsupportedEncodingException e) {
                    // This should not happen with UTF-8
                    e.printStackTrace();
                }
            }
        }
        return result;
    }
    
    // A very basic parser for simple JSON-like strings: {"key":"value",...}
    public static Map<String, String> parseSimpleJson(String json) {
        Map<String, String> result = new HashMap<>();
        // Regex to find "key":"value" pairs
        Pattern p = Pattern.compile("\"(.*?)\"\\s*:\\s*\"(.*?)\"");
        Matcher m = p.matcher(json);
        while (m.find()) {
            result.put(m.group(1), m.group(2));
        }
        return result;
    }
}