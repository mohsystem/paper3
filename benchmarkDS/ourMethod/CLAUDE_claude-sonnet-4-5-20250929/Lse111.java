
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.regex.Pattern;

public class Lse111 {
    // Whitelist pattern: only allow valid hostnames and IP addresses
    // hostname: alphanumeric, dots, hyphens; IP: dotted decimal
    private static final Pattern VALID_HOST_PATTERN = 
        Pattern.compile("^(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)*[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?$|^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    
    private static final int MAX_HOST_LENGTH = 253;
    
    /**
     * Simulates a web endpoint that pings a host.
     * SECURITY: Uses strict input validation to prevent command injection.
     * Only allows validated hostnames/IPs, no shell metacharacters.
     */
    public static String ping(String url) {
        // Input validation: reject null or empty
        if (url == null || url.trim().isEmpty()) {
            return "Error: Invalid input";
        }
        
        url = url.trim();
        
        // Length validation: prevent excessively long inputs
        if (url.length() > MAX_HOST_LENGTH) {
            return "Error: Invalid input";
        }
        
        // Whitelist validation: only allow valid hostnames/IPs
        // This prevents command injection by rejecting shell metacharacters
        if (!VALID_HOST_PATTERN.matcher(url).matches()) {
            return "Error: Invalid input";
        }
        
        StringBuilder output = new StringBuilder();
        Process process = null;
        BufferedReader reader = null;
        
        try {
            // Use ProcessBuilder with individual arguments (no shell interpretation)
            // This prevents command injection as arguments are not passed through a shell
            ProcessBuilder pb = new ProcessBuilder("ping", "-c", "4", url);
            pb.redirectErrorStream(true);
            process = pb.start();
            
            // Read output with proper resource management
            reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            // Wait for process completion with timeout
            if (!process.waitFor(10, java.util.concurrent.TimeUnit.SECONDS)) {
                process.destroyForcibly();
                return "Error: Operation timeout";
            }
            
        } catch (Exception e) {
            // Generic error message - don't leak internal details
            return "Error: Operation failed";
        } finally {
            // Ensure resources are cleaned up
            if (reader != null) {
                try { reader.close(); } catch (Exception ignored) {}
            }
            if (process != null && process.isAlive()) {
                process.destroyForcibly();
            }
        }
        
        return output.toString();
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid hostname
        System.out.println("Test 1 - Valid hostname:");
        System.out.println(ping("localhost"));
        
        // Test case 2: Valid IP address
        System.out.println("\\nTest 2 - Valid IP:");
        System.out.println(ping("127.0.0.1"));
        
        // Test case 3: Command injection attempt (rejected)
        System.out.println("\\nTest 3 - Injection attempt:");
        System.out.println(ping("localhost; cat /etc/passwd"));
        
        // Test case 4: Invalid input with special characters
        System.out.println("\\nTest 4 - Special characters:");
        System.out.println(ping("host`whoami`"));
        
        // Test case 5: Empty input
        System.out.println("\\nTest 5 - Empty input:");
        System.out.println(ping(""));
    }
}
