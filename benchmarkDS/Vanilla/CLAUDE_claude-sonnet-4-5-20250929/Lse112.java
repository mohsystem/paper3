
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URLDecoder;
import java.util.HashMap;
import java.util.Map;

class Lse112 {
    private static Map<String, RouteHandler> routes = new HashMap<>();
    
    interface RouteHandler {
        String handle(String param);
    }
    
    // Decorator pattern - register routes
    public static void route(String path, RouteHandler handler) {
        routes.put(path, handler);
    }
    
    // Ping function that executes system command
    public static String ping(String url) {
        try {
            // Decode URL if needed
            url = URLDecoder.decode(url, "UTF-8");
            
            // Run ping command (limit to 4 packets for safety)
            String os = System.getProperty("os.name").toLowerCase();
            String command;
            
            if (os.contains("win")) {
                command = "ping -n 4 " + url;
            } else {
                command = "ping -c 4 " + url;
            }
            
            Process process = Runtime.getRuntime().exec(command);
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            StringBuilder output = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            process.waitFor();
            reader.close();
            
            return output.toString();
            
        } catch (Exception e) {
            return "Error executing ping: " + e.getMessage();
        }
    }
    
    // Simulate request handling
    public static String handleRequest(String path, String param) {
        RouteHandler handler = routes.get(path);
        if (handler != null) {
            return handler.handle(param);
        }
        return "404 Not Found";
    }
    
    public static void main(String[] args) {
        // Register the ping route (decorator pattern)
        route("/ping", url -> ping(url));
        
        // Test cases
        System.out.println("Test Case 1: Ping localhost");
        System.out.println(handleRequest("/ping", "localhost"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 2: Ping 127.0.0.1");
        System.out.println(handleRequest("/ping", "127.0.0.1"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 3: Ping google.com");
        System.out.println(handleRequest("/ping", "google.com"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 4: Ping invalid URL");
        System.out.println(handleRequest("/ping", "invalid-url-12345.com"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 5: Invalid route");
        System.out.println(handleRequest("/invalid", "test"));
    }
}
