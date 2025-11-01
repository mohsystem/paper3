
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import org.json.JSONObject;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse82 {
    public static String getNameFromPayload(String payload) {
        try {
            JSONObject jsonObject = new JSONObject(payload);
            return jsonObject.optString("name", "Name not found");
        } catch (Exception e) {
            return "Invalid payload";
        }
    }

    public static Map<String, String> queryToMap(String query) {
        Map<String, String> result = new HashMap<>();
        if (query == null || query.isEmpty()) {
            return result;
        }
        for (String param : query.split("&")) {
            String[] pair = param.split("=");
            if (pair.length > 1) {
                result.put(pair[0], URLDecoder.decode(pair[1], StandardCharsets.UTF_8));
            }
        }
        return result;
    }

    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Cases:");
        System.out.println("1. " + getNameFromPayload("{\\"name\\":\\"John\\"}"));
        System.out.println("2. " + getNameFromPayload("{\\"name\\":\\"Alice\\",\\"age\\":30}"));
        System.out.println("3. " + getNameFromPayload("{\\"age\\":25}"));
        System.out.println("4. " + getNameFromPayload("invalid json"));
        System.out.println("5. " + getNameFromPayload("{\\"name\\":\\"\\"}"));

        // Create web server
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/api", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String query = exchange.getRequestURI().getQuery();
                Map<String, String> params = queryToMap(query);
                String payload = params.getOrDefault("payload", "{}");
                
                String response = getNameFromPayload(payload);
                exchange.sendResponseHeaders(200, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        });
        server.setExecutor(null);
        server.start();
        System.out.println("\\nServer started on port 8000");
        System.out.println("Access: http://localhost:8000/api?payload={\\"name\\":\\"John\\"}");
    }
}
