
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.json.JSONObject;
import org.json.JSONException;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse82 {
    private static final int MAX_PAYLOAD_SIZE = 10000;

    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/api", new ApiHandler());
        server.setExecutor(null);
        System.out.println("Server started on port 8000");
        
        // Test cases
        System.out.println("\\nTest Cases:");
        System.out.println("1. Valid: http://localhost:8000/api?payload={\\"name\\":\\"John\\"}");
        System.out.println("2. Valid: http://localhost:8000/api?payload={\\"name\\":\\"Alice\\",\\"age\\":30}");
        System.out.println("3. Missing name: http://localhost:8000/api?payload={\\"age\\":25}");
        System.out.println("4. Invalid JSON: http://localhost:8000/api?payload={invalid}");
        System.out.println("5. Missing payload: http://localhost:8000/api");
        
        server.start();
    }

    static class ApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "";
            int statusCode = 200;

            try {
                String query = exchange.getRequestURI().getQuery();
                Map<String, String> params = parseQuery(query);
                
                String payload = params.get("payload");
                if (payload == null || payload.isEmpty()) {
                    response = "{\\"error\\":\\"Missing payload parameter\\"}";
                    statusCode = 400;
                } else if (payload.length() > MAX_PAYLOAD_SIZE) {
                    response = "{\\"error\\":\\"Payload too large\\"}";
                    statusCode = 413;
                } else {
                    JSONObject jsonObject = new JSONObject(payload);
                    if (jsonObject.has("name")) {
                        String name = jsonObject.getString("name");
                        response = "{\\"name\\":\\"" + escapeJson(name) + "\\"}";
                    } else {
                        response = "{\\"error\\":\\"Name field not found\\"}";
                        statusCode = 400;
                    }
                }
            } catch (JSONException e) {
                response = "{\\"error\\":\\"Invalid JSON format\\"}";
                statusCode = 400;
            } catch (Exception e) {
                response = "{\\"error\\":\\"Internal server error\\"}";
                statusCode = 500;
            }

            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }

        private Map<String, String> parseQuery(String query) {
            Map<String, String> result = new HashMap<>();
            if (query != null) {
                for (String param : query.split("&")) {
                    String[] pair = param.split("=", 2);
                    if (pair.length == 2) {
                        result.put(URLDecoder.decode(pair[0], StandardCharsets.UTF_8),
                                 URLDecoder.decode(pair[1], StandardCharsets.UTF_8));
                    }
                }
            }
            return result;
        }

        private String escapeJson(String str) {
            return str.replace("\\\\", "\\\\\\\\")\n                     .replace("\\"", "\\\\\\"")
                     .replace("\\n", "\\\\n")
                     .replace("\\r", "\\\\r");
        }
    }
}
