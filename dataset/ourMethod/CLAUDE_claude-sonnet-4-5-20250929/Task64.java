
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import org.json.JSONObject;
import org.json.JSONException;

public class Task64 {
    // Maximum allowed response size (10 MB)
    private static final int MAX_RESPONSE_SIZE = 10 * 1024 * 1024;
    
    /**
     * Validate URL format and enforce HTTPS only.
     */
    private static boolean validateUrl(String url) {
        if (url == null || url.isEmpty()) {
            return false;
        }
        
        if (url.length() < 8 || url.length() > 2048) {
            return false;
        }
        
        // Enforce HTTPS only for security
        return url.startsWith("https://");
    }
    
    /**
     * Fetch JSON from a URL with secure HTTPS settings.
     * 
     * @param urlString HTTPS URL to fetch JSON from
     * @return Parsed JSON object or null on error
     */
    public static JSONObject fetchJsonFromUrl(String urlString) {
        // Validate URL before any network operation
        if (!validateUrl(urlString)) {
            System.err.println("Error: Invalid or insecure URL. Only HTTPS URLs are allowed.");
            return null;
        }
        
        HttpsURLConnection connection = null;
        
        try {
            URL url = new URL(urlString);
            
            // Ensure we're using HTTPS
            if (!"https".equalsIgnoreCase(url.getProtocol())) {
                System.err.println("Error: Only HTTPS protocol is allowed.");
                return null;
            }
            
            connection = (HttpsURLConnection) url.openConnection();
            
            // Security: Configure SSL/TLS with minimum TLS 1.2
            SSLContext sslContext = SSLContext.getInstance("TLSv1.2");
            sslContext.init(null, null, null);
            connection.setSSLSocketFactory(sslContext.getSocketFactory());
            
            // Security: Enable hostname verification
            connection.setHostnameVerifier(HttpsURLConnection.getDefaultHostnameVerifier());
            
            // Set request method and timeouts
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(10000); // 10 seconds
            connection.setReadTimeout(30000);    // 30 seconds
            
            // Security: Disable automatic redirects to manually control them
            connection.setInstanceFollowRedirects(false);
            
            // Set User-Agent
            connection.setRequestProperty("User-Agent", "Java-JSON-Fetcher/1.0");
            
            // Check response code
            int responseCode = connection.getResponseCode();
            
            // Handle redirects manually (max 3)
            int redirectCount = 0;
            while ((responseCode == HttpURLConnection.HTTP_MOVED_PERM || 
                    responseCode == HttpURLConnection.HTTP_MOVED_TEMP ||
                    responseCode == HttpURLConnection.HTTP_SEE_OTHER) && 
                   redirectCount < 3) {
                
                String newUrl = connection.getHeaderField("Location");
                if (!validateUrl(newUrl)) {
                    System.err.println("Error: Redirect to non-HTTPS URL blocked.");
                    return null;
                }
                
                connection.disconnect();
                connection = (HttpsURLConnection) new URL(newUrl).openConnection();
                connection.setSSLSocketFactory(sslContext.getSocketFactory());
                connection.setRequestMethod("GET");
                connection.setConnectTimeout(10000);
                connection.setReadTimeout(30000);
                connection.setInstanceFollowRedirects(false);
                
                responseCode = connection.getResponseCode();
                redirectCount++;
            }
            
            if (responseCode != HttpURLConnection.HTTP_OK) {
                System.err.println("Error: HTTP request failed with code: " + responseCode);
                return null;
            }
            
            // Check content length
            int contentLength = connection.getContentLength();
            if (contentLength > MAX_RESPONSE_SIZE) {
                System.err.println("Error: Response size exceeds maximum allowed size.");
                return null;
            }
            
            // Read response with size limit
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] chunk = new byte[8192];
            int bytesRead;
            int totalBytesRead = 0;
            
            try (InputStream inputStream = connection.getInputStream()) {
                while ((bytesRead = inputStream.read(chunk)) != -1) {
                    totalBytesRead += bytesRead;
                    
                    if (totalBytesRead > MAX_RESPONSE_SIZE) {
                        System.err.println("Error: Response size exceeds maximum allowed size.");
                        return null;
                    }
                    
                    buffer.write(chunk, 0, bytesRead);
                }
            }
            
            // Parse JSON
            String jsonString = buffer.toString(StandardCharsets.UTF_8.name());
            JSONObject jsonObject = new JSONObject(jsonString);
            
            return jsonObject;
            
        } catch (JSONException e) {
            System.err.println("Error: Invalid JSON format: " + e.getMessage());
            return null;
        } catch (java.net.MalformedURLException e) {
            System.err.println("Error: Malformed URL: " + e.getMessage());
            return null;
        } catch (java.io.IOException e) {
            System.err.println("Error: I/O error occurred: " + e.getMessage());
            return null;
        } catch (Exception e) {
            System.err.println("Error: Unexpected error occurred: " + e.getClass().getSimpleName());
            return null;
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases when no URL provided
        if (args.length < 1) {
            System.out.println("Running test cases...\\n");
            
            // Test case 1: Valid HTTPS URL
            System.out.println("Test 1: Valid HTTPS URL");
            JSONObject result1 = fetchJsonFromUrl("https://jsonplaceholder.typicode.com/posts/1");
            if (result1 != null) {
                System.out.println("Success: Fetched JSON data");
                String data1 = result1.toString();
                System.out.println("Data: " + data1.substring(0, Math.min(100, data1.length())) + "...\\n");
            } else {
                System.out.println("Failed to fetch data\\n");
            }
            
            // Test case 2: Invalid URL (HTTP)
            System.out.println("Test 2: Invalid URL (HTTP not allowed)");
            JSONObject result2 = fetchJsonFromUrl("http://example.com/data.json");
            if (result2 != null) {
                System.out.println("Unexpected success\\n");
            } else {
                System.out.println("Correctly rejected HTTP URL\\n");
            }
            
            // Test case 3: Null URL
            System.out.println("Test 3: Null URL");
            JSONObject result3 = fetchJsonFromUrl(null);
            if (result3 != null) {
                System.out.println("Unexpected success\\n");
            } else {
                System.out.println("Correctly rejected null URL\\n");
            }
            
            // Test case 4: Empty URL
            System.out.println("Test 4: Empty URL");
            JSONObject result4 = fetchJsonFromUrl("");
            if (result4 != null) {
                System.out.println("Unexpected success\\n");
            } else {
                System.out.println("Correctly rejected empty URL\\n");
            }
            
            // Test case 5: Another valid HTTPS URL
            System.out.println("Test 5: Another valid HTTPS URL");
            JSONObject result5 = fetchJsonFromUrl("https://jsonplaceholder.typicode.com/users/1");
            if (result5 != null) {
                System.out.println("Success: Fetched JSON data");
                String data5 = result5.toString();
                System.out.println("Data: " + data5.substring(0, Math.min(100, data5.length())) + "...\\n");
            } else {
                System.out.println("Failed to fetch data\\n");
            }
            
            return;
        }
        
        // Validate command line argument length
        String url = args[0];
        if (url.length() > 2048) {
            System.err.println("Error: URL too long.");
            System.exit(1);
        }
        
        // Fetch JSON from provided URL
        JSONObject jsonData = fetchJsonFromUrl(url);
        
        if (jsonData != null) {
            System.out.println("Successfully fetched JSON:");
            System.out.println(jsonData.toString(2)); // Pretty print with indent
        } else {
            System.err.println("Failed to fetch JSON from URL.");
            System.exit(1);
        }
    }
}
