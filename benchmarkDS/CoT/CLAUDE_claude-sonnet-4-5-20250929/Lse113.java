
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.regex.Pattern;

public class Lse113 {
    private static final Pattern VALID_HOSTNAME = Pattern.compile(
        "^([a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?\\\\.)+[a-zA-Z]{2,}$"
    );
    private static final Pattern VALID_IP = Pattern.compile(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
    );
    
    public static String ping(String url) {
        // Input validation and sanitization
        if (url == null || url.trim().isEmpty()) {
            return "Error: Invalid input - URL cannot be empty";
        }
        
        url = url.trim();
        
        // Remove protocol if present
        url = url.replaceAll("^https?://", "");
        // Remove path if present
        url = url.split("/")[0];
        
        // Validate hostname or IP
        if (!VALID_HOSTNAME.matcher(url).matches() && !VALID_IP.matcher(url).matches()) {
            return "Error: Invalid hostname or IP address format";
        }
        
        try {
            // Use Java's InetAddress for safe reachability check (no shell command)
            InetAddress address = InetAddress.getByName(url);
            boolean reachable = address.isReachable(5000); // 5 second timeout
            
            if (reachable) {
                return "URL " + url + " is alive";
            } else {
                return "URL " + url + " is not alive";
            }
        } catch (UnknownHostException e) {
            return "Error: Unknown host - " + url;
        } catch (IOException e) {
            return "Error: Cannot reach host - " + url;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + ping("8.8.8.8"));
        System.out.println("Test 2: " + ping("google.com"));
        System.out.println("Test 3: " + ping("invalid..host"));
        System.out.println("Test 4: " + ping("; rm -rf /"));
        System.out.println("Test 5: " + ping(""));
    }
}
