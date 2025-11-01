
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
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
    public static String processRequest(String payload) {
        if (payload == null || payload.isEmpty()) {
            return "Error: Missing payload parameter";
        }
        
        try {
            Yaml yaml = new Yaml(new SafeConstructor());
            Map<String, Object> data = yaml.load(payload);
            
            if (data != null && data.containsKey("type")) {
                String type = data.get("type").toString();
                if ("Create".equals(type)) {
                    return "Error: Create type is not allowed";
                }
            }
            
            return "Success: Payload processed";
        } catch (Exception e) {
            return "Error: Invalid YAML format";
        }
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Case 1: " + processRequest(null));
        System.out.println("Test Case 2: " + processRequest(""));
        System.out.println("Test Case 3: " + processRequest("type: Create"));
        System.out.println("Test Case 4: " + processRequest("type: Update"));
        System.out.println("Test Case 5: " + processRequest("name: test\\nvalue: 123"));
        
        // Start web server
        HttpServer server = HttpServer.create(new InetSocketAddress(5000), 0);
        server.createContext("/", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String query = exchange.getRequestURI().getQuery();
                String payload = null;
                
                if (query != null) {
                    String[] params = query.split("&");
                    for (String param : params) {
                        String[] keyValue = param.split("=");
                        if (keyValue.length == 2 && "payload".equals(keyValue[0])) {
                            payload = URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8);
                        }
                    }
                }
                
                String response = processRequest(payload);
                exchange.sendResponseHeaders(200, response.getBytes().length);
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        });
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 5000");
    }
}
