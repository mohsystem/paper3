
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse112 {
    // Validates URL to prevent command injection
    private static boolean isValidUrl(String url) {
        if (url == null || url.trim().isEmpty()) {
            return false;
        }
        
        // Only allow alphanumeric, dots, hyphens, and colons (for IPv6 and ports)
        String urlPattern = "^[a-zA-Z0-9.-:]+$";
        return Pattern.matches(urlPattern, url);
    }
    
    public static String ping(String url) {
        try {
            // Decode URL if encoded
            url = URLDecoder.decode(url, StandardCharsets.UTF_8.toString());
            
            // Validate URL to prevent command injection
            if (!isValidUrl(url)) {
                return "Invalid URL format. Only alphanumeric characters, dots, hyphens, and colons are allowed.";
            }
            
            // Limit URL length
            if (url.length() > 253) {
                return "URL too long";
            }
            
            // Build command safely
            ProcessBuilder processBuilder;
            String os = System.getProperty("os.name").toLowerCase();
            
            if (os.contains("win")) {
                processBuilder = new ProcessBuilder("ping", "-n", "4", url);
            } else {
                processBuilder = new ProcessBuilder("ping", "-c", "4", url);
            }
            
            processBuilder.redirectErrorStream(true);
            Process process = processBuilder.start();
            
            // Read output
            StringBuilder output = new StringBuilder();
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\\n");
            }
            
            // Wait for process to complete with timeout
            process.waitFor();
            reader.close();
            
            return "Ping results for " + url + ":\\n" + output.toString();
            
        } catch (Exception e) {
            return "Error executing ping: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: Valid domain");
        System.out.println(ping("google.com"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test 2: Valid IP address");
        System.out.println(ping("8.8.8.8"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test 3: Localhost");
        System.out.println(ping("localhost"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test 4: Invalid URL with command injection attempt");
        System.out.println(ping("google.com; ls"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test 5: Invalid URL with special characters");
        System.out.println(ping("google.com && whoami"));
    }
}
