
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import javax.net.ssl.HttpsURLConnection;

public class Task76 {
    private static final int TIMEOUT = 10000; // 10 seconds
    private static final int MAX_CONTENT_LENGTH = 5 * 1024 * 1024; // 5MB
    private static final Pattern TITLE_PATTERN = Pattern.compile("<title[^>]*>\\\\s*([^<]+)\\\\s*</title>", Pattern.CASE_INSENSITIVE);
    
    public static String extractPageTitle(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            return "Error: URL cannot be empty";
        }
        
        // Validate URL format and ensure HTTPS
        if (!isValidHttpsUrl(urlString)) {
            return "Error: Invalid URL or non-HTTPS URL provided";
        }
        
        HttpsURLConnection connection = null;
        BufferedReader reader = null;
        
        try {
            // Create URL and validate
            URI uri = new URI(urlString);
            URL url = uri.toURL();
            
            // Establish secure connection
            connection = (HttpsURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(TIMEOUT);
            connection.setReadTimeout(TIMEOUT);
            connection.setInstanceFollowRedirects(false); // Prevent redirect attacks
            
            // Set secure headers
            connection.setRequestProperty("User-Agent", "SecurePageTitleExtractor/1.0");
            connection.setRequestProperty("Accept", "text/html");
            
            // Check response code
            int responseCode = connection.getResponseCode();
            if (responseCode != 200) {
                return "Error: HTTP " + responseCode;
            }
            
            // Check content length
            int contentLength = connection.getContentLength();
            if (contentLength > MAX_CONTENT_LENGTH) {
                return "Error: Content too large";
            }
            
            // Read response
            reader = new BufferedReader(new InputStreamReader(connection.getInputStream(), "UTF-8"));
            StringBuilder content = new StringBuilder();
            String line;
            int totalRead = 0;
            
            while ((line = reader.readLine()) != null) {
                totalRead += line.length();
                if (totalRead > MAX_CONTENT_LENGTH) {
                    return "Error: Content exceeds maximum size";
                }
                content.append(line).append("\\n");
                
                // Early extraction if title found
                if (content.indexOf("</title>") != -1) {
                    break;
                }
            }
            
            // Extract title
            return extractTitleFromHtml(content.toString());
            
        } catch (URISyntaxException e) {
            return "Error: Invalid URL syntax - " + sanitizeErrorMessage(e.getMessage());
        } catch (Exception e) {
            return "Error: " + sanitizeErrorMessage(e.getMessage());
        } finally {
            try {
                if (reader != null) reader.close();
                if (connection != null) connection.disconnect();
            } catch (Exception e) {
                // Ignore cleanup errors
            }
        }
    }
    
    private static boolean isValidHttpsUrl(String url) {
        if (url == null || !url.toLowerCase().startsWith("https://")) {
            return false;
        }
        
        try {
            URI uri = new URI(url);
            String host = uri.getHost();
            
            // Prevent SSRF to internal networks
            if (host == null || host.isEmpty()) {
                return false;
            }
            
            // Block localhost and private IP ranges
            if (host.equals("localhost") || host.equals("127.0.0.1") || 
                host.equals("::1") || host.startsWith("192.168.") || 
                host.startsWith("10.") || host.startsWith("172.")) {
                return false;
            }
            
            return true;
        } catch (URISyntaxException e) {
            return false;
        }
    }
    
    private static String extractTitleFromHtml(String html) {
        if (html == null || html.isEmpty()) {
            return "No title found";
        }
        
        Matcher matcher = TITLE_PATTERN.matcher(html);
        if (matcher.find()) {
            String title = matcher.group(1);
            // Sanitize title to prevent injection
            return sanitizeOutput(title);
        }
        
        return "No title found";
    }
    
    private static String sanitizeOutput(String text) {
        if (text == null) return "";
        // Remove HTML entities and trim
        return text.replaceAll("&[a-zA-Z0-9#]+;", "")
                   .replaceAll("[\\\\r\\\\n]+", " ")
                   .trim();
    }
    
    private static String sanitizeErrorMessage(String message) {
        if (message == null) return "Unknown error";
        // Remove sensitive information from error messages
        return message.replaceAll("https?://[^\\\\s]+", "[URL]")
                      .substring(0, Math.min(message.length(), 100));
    }
    
    public static void main(String[] args) {
        if (args.length == 0) {
            System.out.println("Usage: java Task76 <https-url>");
            System.out.println("\\nTest cases:");
            
            // Test cases
            String[] testUrls = {
                "https://www.example.com",
                "https://www.google.com",
                "http://www.example.com", // Should fail - not HTTPS
                "https://localhost", // Should fail - localhost blocked
                "invalid-url" // Should fail - invalid format
            };
            
            for (String url : testUrls) {
                System.out.println("\\nURL: " + url);
                System.out.println("Title: " + extractPageTitle(url));
            }
        } else {
            String result = extractPageTitle(args[0]);
            System.out.println(result);
        }
    }
}
