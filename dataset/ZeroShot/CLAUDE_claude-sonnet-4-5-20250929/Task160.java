
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.Scanner;

public class Task160 {
    private static final Set<String> ALLOWED_PROTOCOLS = new HashSet<>(Arrays.asList("http", "https"));
    private static final Set<String> BLOCKED_HOSTS = new HashSet<>(Arrays.asList(
        "localhost", "127.0.0.1", "0.0.0.0", "::1", "169.254.169.254"
    ));
    private static final int TIMEOUT = 5000;
    private static final int MAX_RESPONSE_SIZE = 1048576; // 1MB

    public static String makeHttpRequest(String urlString) {
        try {
            // Validate and sanitize URL
            URL url = new URL(urlString);
            
            // Check protocol
            if (!ALLOWED_PROTOCOLS.contains(url.getProtocol().toLowerCase())) {
                return "Error: Only HTTP and HTTPS protocols are allowed";
            }
            
            // Check for blocked hosts (SSRF protection)
            String host = url.getHost().toLowerCase();
            if (BLOCKED_HOSTS.contains(host) || host.startsWith("192.168.") || 
                host.startsWith("10.") || host.startsWith("172.")) {
                return "Error: Access to internal/private network addresses is blocked";
            }
            
            // Make request
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(TIMEOUT);
            connection.setReadTimeout(TIMEOUT);
            connection.setInstanceFollowRedirects(false); // Prevent redirect-based SSRF
            
            int responseCode = connection.getResponseCode();
            
            BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            StringBuilder response = new StringBuilder();
            String inputLine;
            int totalBytes = 0;
            
            while ((inputLine = in.readLine()) != null) {
                totalBytes += inputLine.length();
                if (totalBytes > MAX_RESPONSE_SIZE) {
                    in.close();
                    return "Error: Response size exceeds limit";
                }
                response.append(inputLine).append("\\n");
            }
            in.close();
            
            return "Status Code: " + responseCode + "\\nResponse:\\n" + response.toString();
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        System.out.println("HTTP Request Maker - Test Cases\\n");
        
        // Test case 1: Valid HTTPS URL
        System.out.println("Test 1: Valid HTTPS URL");
        String result1 = makeHttpRequest("https://www.example.com");
        System.out.println(result1.substring(0, Math.min(200, result1.length())) + "...\\n");
        
        // Test case 2: Invalid protocol
        System.out.println("Test 2: Invalid protocol (file://)");
        String result2 = makeHttpRequest("file:///etc/passwd");
        System.out.println(result2 + "\\n");
        
        // Test case 3: Blocked host (localhost)
        System.out.println("Test 3: Blocked host (localhost)");
        String result3 = makeHttpRequest("http://localhost:8080");
        System.out.println(result3 + "\\n");
        
        // Test case 4: Blocked private IP
        System.out.println("Test 4: Blocked private IP");
        String result4 = makeHttpRequest("http://192.168.1.1");
        System.out.println(result4 + "\\n");
        
        // Test case 5: Invalid URL
        System.out.println("Test 5: Invalid URL");
        String result5 = makeHttpRequest("not-a-valid-url");
        System.out.println(result5 + "\\n");
    }
}
