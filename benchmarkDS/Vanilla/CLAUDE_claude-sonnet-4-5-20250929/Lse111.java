
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URLDecoder;
import java.util.HashMap;
import java.util.Map;

public class Lse111 {
    
    // Simulated web framework decorator functionality
    private static Map<String, RouteHandler> routes = new HashMap<>();
    
    interface RouteHandler {
        String handle(String url);
    }
    
    // Register route (simulates decorator)
    public static void route(String path, RouteHandler handler) {
        routes.put(path, handler);
    }
    
    // Ping function that executes system ping command
    public static String ping(String url) {
        try {
            String command;
            String os = System.getProperty("os.name").toLowerCase();
            
            if (os.contains("win")) {
                command = "ping -n 4 " + url;
            } else {
                command = "ping -c 4 " + url;
            }
            
            Process process = Runtime.getRuntime().exec(command);
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
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
    
    public static void main(String[] args) {
        // Register the route
        route("/ping", (url) -> ping(url));
        
        // Test cases
        String[] testUrls = {
            "localhost",
            "127.0.0.1",
            "google.com",
            "8.8.8.8",
            "example.com"
        };
        
        System.out.println("Testing ping functionality:\\n");
        for (String testUrl : testUrls) {
            System.out.println("=== Pinging: " + testUrl + " ===");
            String result = routes.get("/ping").handle(testUrl);
            System.out.println(result);
            System.out.println();
        }
    }
}
