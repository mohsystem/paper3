
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.yaml.snakeyaml.LoaderOptions;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.SafeConstructor;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse84 {
    // Maximum payload size to prevent DoS attacks
    private static final int MAX_PAYLOAD_SIZE = 10240; // 10KB

    public static void main(String[] args) throws IOException {
        // Create HTTP server on port 5000
        HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
        server.createContext("/", new RequestHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 5000");

        // Test cases
        runTestCases();
    }

    static class RequestHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode;

            try {
                // Only accept POST requests for security
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    response = "Error: Method not allowed";
                    statusCode = 405;
                } else {
                    // Parse query parameters
                    String query = exchange.getRequestURI().getQuery();
                    Map<String, String> params = parseQuery(query);

                    // Check if payload parameter exists
                    if (!params.containsKey("payload")) {
                        response = "Error: Missing payload parameter";
                        statusCode = 400;
                    } else {
                        String payload = params.get("payload");
                        
                        // Validate payload size to prevent DoS
                        if (payload.length() > MAX_PAYLOAD_SIZE) {
                            response = "Error: Payload too large";
                            statusCode = 400;
                        } else {
                            response = processPayload(payload);
                            statusCode = 200;
                        }
                    }
                }
            } catch (Exception e) {
                // Generic error message - don't leak internal details\n                response = "Error: Invalid request";\n                statusCode = 400;\n            }\n\n            // Send response\n            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);\n            OutputStream os = exchange.getResponseBody();\n            os.write(response.getBytes(StandardCharsets.UTF_8));\n            os.close();\n        }\n    }\n\n    private static Map<String, String> parseQuery(String query) {\n        Map<String, String> params = new HashMap<>();\n        if (query == null || query.isEmpty()) {\n            return params;\n        }\n\n        // Parse query parameters safely\n        String[] pairs = query.split("&");\n        for (String pair : pairs) {\n            int idx = pair.indexOf("=");\n            if (idx > 0 && idx < pair.length() - 1) {\n                try {\n                    String key = URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8.name());\n                    String value = URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8.name());\n                    params.put(key, value);\n                } catch (Exception e) {\n                    // Skip malformed parameters\n                }\n            }\n        }\n        return params;\n    }\n\n    private static String processPayload(String payload) {\n        try {\n            // Use SafeConstructor to prevent arbitrary code execution\n            // This blocks instantiation of arbitrary Java objects\n            LoaderOptions loaderOptions = new LoaderOptions();\n            loaderOptions.setMaxAliasesForCollections(50); // Prevent billion laughs\n            loaderOptions.setAllowDuplicateKeys(false);\n            \n            Yaml yaml = new Yaml(new SafeConstructor(loaderOptions));\n            \n            // Parse YAML safely - only allows basic types\n            Object data = yaml.load(payload);\n            \n            // Validate parsed data is a Map\n            if (!(data instanceof Map)) {\n                return "Error: Invalid payload format";\n            }\n            \n            @SuppressWarnings("unchecked")\n            Map<String, Object> dataMap = (Map<String, Object>) data;\n            \n            // Check if type field exists and equals "Create"\n            Object type = dataMap.get("type");\n            if (type != null && "Create".equals(type.toString())) {\n                return "Error: Create type not allowed";\n            }\n            \n            return "Success: Payload processed";\n            \n        } catch (Exception e) {\n            // Don't leak internal details in error messages
            return "Error: Invalid YAML format";
        }
    }

    private static void runTestCases() {
        System.out.println("\\n=== Running Test Cases ===");
        
        // Test 1: Valid payload with type=Create
        System.out.println("\\nTest 1: Valid YAML with type=Create");
        System.out.println(processPayload("type: Create\\ndata: test"));
        
        // Test 2: Valid payload with different type
        System.out.println("\\nTest 2: Valid YAML with type=Update");
        System.out.println(processPayload("type: Update\\ndata: test"));
        
        // Test 3: Valid payload without type field
        System.out.println("\\nTest 3: Valid YAML without type field");
        System.out.println(processPayload("data: test\\nvalue: 123"));
        
        // Test 4: Invalid YAML syntax
        System.out.println("\\nTest 4: Invalid YAML syntax");
        System.out.println(processPayload("invalid: [yaml: syntax"));
        
        // Test 5: Empty payload
        System.out.println("\\nTest 5: Empty payload");
        System.out.println(processPayload(""));
    }
}
