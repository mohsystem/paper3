
import java.io.*;
import java.net.*;
import java.util.regex.*;

public class Task160 {
    private static final int TIMEOUT = 5000; // 5 seconds
    private static final int MAX_RESPONSE_SIZE = 1048576; // 1MB
    
    public static String makeHttpRequest(String urlString) {
        // Validate URL format
        if (urlString == null || urlString.trim().isEmpty()) {
            return "Error: URL cannot be empty";
        }
        
        try {
            URL url = new URL(urlString);
            
            // Security: Only allow HTTP and HTTPS protocols
            String protocol = url.getProtocol().toLowerCase();
            if (!protocol.equals("http") && !protocol.equals("https")) {
                return "Error: Only HTTP and HTTPS protocols are allowed";
            }
            
            // Security: Block requests to private/internal IP addresses
            String host = url.getHost().toLowerCase();
            if (isPrivateOrLocalAddress(host)) {
                return "Error: Requests to private/internal addresses are not allowed";
            }
            
            // Make HTTP request with security settings
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(TIMEOUT);
            connection.setReadTimeout(TIMEOUT);
            connection.setInstanceFollowRedirects(false); // Disable auto redirects
            connection.setRequestProperty("User-Agent", "SecureHttpClient/1.0");
            
            int responseCode = connection.getResponseCode();
            
            // Read response with size limit
            StringBuilder response = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(
                    new InputStreamReader(connection.getInputStream()))) {
                String line;
                int totalBytes = 0;
                while ((line = reader.readLine()) != null) {
                    totalBytes += line.length();
                    if (totalBytes > MAX_RESPONSE_SIZE) {
                        return "Error: Response size exceeds limit";
                    }
                    response.append(line).append("\\n");
                }
            }
            
            return "Status: " + responseCode + "\\n" + response.toString();
            
        } catch (MalformedURLException e) {
            return "Error: Invalid URL format - " + e.getMessage();
        } catch (SocketTimeoutException e) {
            return "Error: Request timeout";
        } catch (IOException e) {
            return "Error: " + e.getMessage();
        }
    }
    
    private static boolean isPrivateOrLocalAddress(String host) {
        // Block localhost, private IPs, and link-local addresses
        String[] blockedPatterns = {
            "localhost", "127\\\\..*", "0\\\\.0\\\\.0\\\\.0",
            "10\\\\..*", "172\\\\.(1[6-9]|2[0-9]|3[0-1])\\\\..*", "192\\\\.168\\\\..*",
            "169\\\\.254\\\\..*", "::1", "fc00:.*", "fe80:.*"
        };
        
        for (String pattern : blockedPatterns) {
            if (host.matches(pattern)) {
                return true;
            }
        }
        
        try {
            InetAddress addr = InetAddress.getByName(host);
            return addr.isLoopbackAddress() || addr.isLinkLocalAddress() || 
                   addr.isSiteLocalAddress();
        } catch (UnknownHostException e) {
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Valid HTTPS URL ===");
        System.out.println(makeHttpRequest("https://www.example.com"));
        
        System.out.println("\\n=== Test Case 2: Invalid Protocol ===");
        System.out.println(makeHttpRequest("ftp://example.com"));
        
        System.out.println("\\n=== Test Case 3: Localhost (blocked) ===");
        System.out.println(makeHttpRequest("http://localhost:8080"));
        
        System.out.println("\\n=== Test Case 4: Private IP (blocked) ===");
        System.out.println(makeHttpRequest("http://192.168.1.1"));
        
        System.out.println("\\n=== Test Case 5: Empty URL ===");
        System.out.println(makeHttpRequest(""));
    }
}
