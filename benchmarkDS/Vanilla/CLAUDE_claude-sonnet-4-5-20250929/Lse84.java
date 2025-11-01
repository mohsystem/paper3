
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.yaml.YAMLFactory;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse84 {
    private static final ObjectMapper yamlMapper = new ObjectMapper(new YAMLFactory());

    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
        server.createContext("/", Lse84::handleRequest);
        server.setExecutor(null);
        System.out.println("Server started on port 5000");
        server.start();
    }

    private static void handleRequest(HttpExchange exchange) throws IOException {
        String query = exchange.getRequestURI().getQuery();
        String response;

        if (query == null || !query.contains("payload=")) {
            response = "Error: Missing payload parameter";
        } else {
            Map<String, String> params = parseQuery(query);
            String payload = params.get("payload");
            response = processPayload(payload);
        }

        exchange.sendResponseHeaders(200, response.getBytes().length);
        OutputStream os = exchange.getResponseBody();
        os.write(response.getBytes());
        os.close();
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> result = new HashMap<>();
        if (query != null) {
            String[] pairs = query.split("&");
            for (String pair : pairs) {
                String[] keyValue = pair.split("=", 2);
                if (keyValue.length == 2) {
                    try {
                        result.put(keyValue[0], URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8.name()));
                    } catch (Exception e) {
                        result.put(keyValue[0], keyValue[1]);
                    }
                }
            }
        }
        return result;
    }

    private static String processPayload(String payload) {
        try {
            Map<String, Object> data = yamlMapper.readValue(payload, Map.class);
            if (data.containsKey("type") && "Create".equals(data.get("type"))) {
                return "Error: Type cannot be Create";
            }
            return "Success: Payload processed";
        } catch (Exception e) {
            return "Error: Invalid YAML format";
        }
    }
}
