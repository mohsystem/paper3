
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

public class Lse113 {
    
    public static String ping(String url) {
        try {
            // Decode URL if needed
            String decodedUrl = URLDecoder.decode(url, StandardCharsets.UTF_8.toString());
            
            // Determine OS and set ping command accordingly
            String os = System.getProperty("os.name").toLowerCase();
            String pingCommand;
            
            if (os.contains("win")) {
                pingCommand = "ping -n 1 " + decodedUrl;
            } else {
                pingCommand = "ping -c 1 " + decodedUrl;
            }
            
            // Execute ping command
            Process process = Runtime.getRuntime().exec(pingCommand);
            
            // Read output
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            StringBuilder pong = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                pong.append(line).append("\\n");
            }
            
            // Wait for process to complete and get exit value
            int exitValue = process.waitFor();
            
            if (exitValue == 0) {
                return "The URL " + decodedUrl + " is alive\\n" + pong.toString();
            } else {
                return "The URL " + decodedUrl + " is not alive\\n" + pong.toString();
            }
            
        } catch (Exception e) {
            return "Error pinging URL: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(ping("google.com"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 2:");
        System.out.println(ping("localhost"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 3:");
        System.out.println(ping("8.8.8.8"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 4:");
        System.out.println(ping("invalid-url-that-does-not-exist-12345.com"));
        System.out.println("\\n" + "=".repeat(50) + "\\n");
        
        System.out.println("Test Case 5:");
        System.out.println(ping("github.com"));
    }
}
