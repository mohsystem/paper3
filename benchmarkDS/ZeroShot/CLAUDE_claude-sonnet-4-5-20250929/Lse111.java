
// Java - SECURE VERSION with input validation
import java.io.*;
import java.net.*;
import java.util.regex.Pattern;

public class Lse111 {
    // Whitelist pattern for valid hostnames/IPs
    private static final Pattern VALID_HOST = Pattern.compile(
        "^([a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?\\\\.)*[a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?$|^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
    );
    
    public static String ping(String url) {
        if (url == null || url.trim().isEmpty()) {
            return "Error: URL cannot be empty";
        }
        
        // Validate input to prevent command injection
        if (!VALID_HOST.matcher(url).matches()) {
            return "Error: Invalid hostname or IP address format";
        }
        
        StringBuilder output = new StringBuilder();
        
        try {
            // Use InetAddress.isReachable() instead of system ping command
            InetAddress address = InetAddress.getByName(url);
            boolean reachable = address.isReachable(5000);
            
            output.append("Ping test for: ").append(url).append("\\n");
            output.append("Host: ").append(address.getHostAddress()).append("\\n");
            output.append("Reachable: ").append(reachable ? "Yes" : "No").append("\\n");
            
            if (reachable) {
                // Perform multiple ping tests
                for (int i = 0; i < 4; i++) {
                    long startTime = System.nanoTime();
                    boolean result = address.isReachable(5000);
                    long endTime = System.nanoTime();
                    long duration = (endTime - startTime) / 1_000_000;
                    
                    output.append("Reply from ").append(address.getHostAddress())
                          .append(": time=").append(duration).append("ms")
                          .append(result ? " (success)" : " (failed)").append("\\n");
                }
            }
            
        } catch (UnknownHostException e) {
            output.append("Error: Unknown host - ").append(url);
        } catch (IOException e) {
            output.append("Error: Unable to reach host - ").append(e.getMessage());
        }
        
        return output.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("=== Test Case 1: Valid domain ===");
        System.out.println(ping("google.com"));
        System.out.println();
        
        System.out.println("=== Test Case 2: Valid IP ===");
        System.out.println(ping("8.8.8.8"));
        System.out.println();
        
        System.out.println("=== Test Case 3: Localhost ===");
        System.out.println(ping("127.0.0.1"));
        System.out.println();
        
        System.out.println("=== Test Case 4: Invalid input (command injection attempt) ===");
        System.out.println(ping("google.com; rm -rf /"));
        System.out.println();
        
        System.out.println("=== Test Case 5: Empty input ===");
        System.out.println(ping(""));
    }
}
