
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.URLConnection;
import java.util.regex.Pattern;

public class Lse113 {
    
    // Validate URL to prevent command injection
    private static boolean isValidUrl(String url) {
        if (url == null || url.isEmpty()) {
            return false;
        }
        
        // Remove http:// or https:// prefix for validation
        String cleanUrl = url.replaceFirst("^https?://", "");
        
        // Check for valid domain/IP pattern (basic validation)
        // Only allow alphanumeric, dots, hyphens, and colons (for ports)
        Pattern pattern = Pattern.compile("^[a-zA-Z0-9.-]+(:[0-9]+)?$");
        return pattern.matcher(cleanUrl).matches();
    }
    
    // Extract hostname from URL
    private static String extractHostname(String url) {
        try {
            String cleanUrl = url;
            if (!url.startsWith("http://") && !url.startsWith("https://")) {
                cleanUrl = "http://" + url;
            }
            URL urlObj = new URL(cleanUrl);
            return urlObj.getHost();
        } catch (Exception e) {
            return url.replaceFirst("^https?://", "").split(":")[0];
        }
    }
    
    public static String ping(String url) {
        // Validate URL to prevent command injection
        if (!isValidUrl(url)) {
            return "Invalid URL format. URL is not alive.";
        }
        
        String hostname = extractHostname(url);
        
        try {
            String os = System.getProperty("os.name").toLowerCase();
            ProcessBuilder processBuilder;
            
            // Adjust ping command based on OS
            if (os.contains("win")) {
                processBuilder = new ProcessBuilder("ping", "-n", "1", hostname);
            } else {
                processBuilder = new ProcessBuilder("ping", "-c", "1", hostname);
            }
            
            Process process = processBuilder.start();
            int exitCode = process.waitFor();
            
            // Read output
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            StringBuilder pong = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                pong.append(line).append("\\n");
            }
            
            if (exitCode == 0) {
                return "URL " + url + " is alive.\\n" + pong.toString();
            } else {
                return "URL " + url + " is not alive.";
            }
            
        } catch (Exception e) {
            return "Error pinging URL " + url + ": " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUrls = {
            "google.com",
            "127.0.0.1",
            "localhost",
            "example.com",
            "invalid-url-that-does-not-exist-12345.com"
        };
        
        System.out.println("=== Ping Test Results ===\\n");
        for (String testUrl : testUrls) {
            System.out.println("Testing: " + testUrl);
            System.out.println(ping(testUrl));
            System.out.println("---");
        }
    }
}
