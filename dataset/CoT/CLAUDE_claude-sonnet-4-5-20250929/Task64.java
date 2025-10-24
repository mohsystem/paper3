
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

public class Task64 {
    private static final int TIMEOUT = 10000; // 10 seconds
    private static final int MAX_SIZE = 10 * 1024 * 1024; // 10 MB
    private static final List<String> ALLOWED_PROTOCOLS = Arrays.asList("https", "http");
    
    public static JsonObject fetchAndParseJson(String urlString) throws Exception {
        // Input validation
        if (urlString == null || urlString.trim().isEmpty()) {
            throw new IllegalArgumentException("URL cannot be null or empty");
        }
        
        // URL validation
        URL url = new URL(urlString);
        String protocol = url.getProtocol().toLowerCase();
        
        if (!ALLOWED_PROTOCOLS.contains(protocol)) {
            throw new SecurityException("Only HTTP and HTTPS protocols are allowed");
        }
        
        // Prevent SSRF attacks - block private IP ranges
        String host = url.getHost().toLowerCase();
        if (host.equals("localhost") || host.equals("127.0.0.1") || 
            host.startsWith("192.168.") || host.startsWith("10.") || 
            host.startsWith("172.16.") || host.equals("0.0.0.0")) {
            throw new SecurityException("Access to private network addresses is not allowed");
        }
        
        HttpURLConnection connection = null;
        BufferedReader reader = null;
        
        try {
            connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(TIMEOUT);
            connection.setReadTimeout(TIMEOUT);
            connection.setInstanceFollowRedirects(false); // Prevent redirect attacks
            
            // Set user agent
            connection.setRequestProperty("User-Agent", "SecureJSONFetcher/1.0");
            
            int responseCode = connection.getResponseCode();
            if (responseCode != HttpURLConnection.HTTP_OK) {
                throw new Exception("HTTP error code: " + responseCode);
            }
            
            // Check content type
            String contentType = connection.getContentType();
            if (contentType != null && !contentType.contains("application/json") && 
                !contentType.contains("text/plain")) {
                throw new SecurityException("Invalid content type: " + contentType);
            }
            
            reader = new BufferedReader(new InputStreamReader(
                connection.getInputStream(), StandardCharsets.UTF_8));
            
            StringBuilder jsonContent = new StringBuilder();
            String line;
            int totalSize = 0;
            
            while ((line = reader.readLine()) != null) {
                totalSize += line.getBytes(StandardCharsets.UTF_8).length;
                if (totalSize > MAX_SIZE) {
                    throw new SecurityException("Response size exceeds maximum allowed size");
                }
                jsonContent.append(line);
            }
            
            // Parse JSON safely
            JsonObject jsonObject = JsonParser.parseString(jsonContent.toString()).getAsJsonObject();
            return jsonObject;
            
        } finally {
            if (reader != null) {
                try { reader.close(); } catch (Exception e) { }
            }
            if (connection != null) {
                connection.disconnect();
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases with safe public URLs
        String[] testUrls = {
            "https://jsonplaceholder.typicode.com/posts/1",
            "https://api.github.com/users/github",
            "https://jsonplaceholder.typicode.com/users/1",
            "https://jsonplaceholder.typicode.com/todos/1",
            "https://jsonplaceholder.typicode.com/comments/1"
        };
        
        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("\\nTest Case " + (i + 1) + ":");
            System.out.println("URL: " + testUrls[i]);
            try {
                JsonObject result = fetchAndParseJson(testUrls[i]);
                System.out.println("Result: " + result.toString());
            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
            }
        }
    }
}
