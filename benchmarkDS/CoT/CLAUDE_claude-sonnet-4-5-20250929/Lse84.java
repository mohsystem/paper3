
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.HashMap;

public class Lse84 {
    private static final int MAX_PAYLOAD_SIZE = 10000;

    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
        server.createContext("/", new RequestHandler());
        server.setExecutor(null);
        System.out.println("Server started on port 5000");
        server.start();

        // Test cases
        System.out.println("\\nTest Cases:");
        testCase1();
        testCase2();
        testCase3();
        testCase4();
        testCase5();
    }

    static class RequestHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            String response;
            int statusCode;

            if (query == null || !query.contains("payload=")) {
                response = "Error: payload parameter is missing";
                statusCode = 400;
            } else {
                String payload = extractPayload(query);
                if (payload == null || payload.isEmpty()) {
                    response = "Error: payload parameter is empty";
                    statusCode = 400;
                } else if (payload.length() > MAX_PAYLOAD_SIZE) {
                    response = "Error: payload too large";
                    statusCode = 400;
                } else {
                    response = processPayload(payload);
                    statusCode = response.startsWith("Error") ? 400 : 200;
                }
            }

            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }

        private String extractPayload(String query) {
            try {
                for (String param : query.split("&")) {
                    String[] pair = param.split("=", 2);
                    if (pair.length == 2 && pair[0].equals("payload")) {
                        return URLDecoder.decode(pair[1], StandardCharsets.UTF_8.name());
                    }
                }
            } catch (Exception e) {
                return null;
            }
            return null;
        }

        private String processPayload(String payload) {
            try {
                Yaml yaml = new Yaml(new SafeConstructor());
                Object data = yaml.load(payload);
                
                if (data instanceof Map) {
                    Map<String, Object> map = (Map<String, Object>) data;
                    if (map.containsKey("type") && "Create".equals(map.get("type"))) {
                        return "Error: Create type is not allowed";
                    }
                    return "Success: Payload processed";
                }
                return "Success: Payload processed";
            } catch (Exception e) {
                return "Error: Invalid YAML format";
            }
        }
    }

    static void testCase1() {
        System.out.println("Test 1 - Missing payload: Expected error");
    }

    static void testCase2() {
        System.out.println("Test 2 - Valid payload with type=Create: Expected error");
    }

    static void testCase3() {
        System.out.println("Test 3 - Valid payload with type=Update: Expected success");
    }

    static void testCase4() {
        System.out.println("Test 4 - Invalid YAML: Expected error");
    }

    static void testCase5() {
        System.out.println("Test 5 - Empty payload: Expected error");
    }
}
