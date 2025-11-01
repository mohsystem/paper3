
import java.net.URI;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

public class Lse33 {
    public static String redirect_invalid_url(String urlString) {
        try {
            URL url = new URL(urlString);
            String host = url.getHost();
            
            if (host != null && host.equals("example.com")) {
                return "Redirect to: " + urlString;
            } else {
                return "Error: Invalid domain. Only example.com is allowed.";
            }
        } catch (Exception e) {
            return "Error: Invalid URL format.";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "http://example.com/page1",
            "https://example.com/secure",
            "http://malicious.com/page",
            "http://subdomain.example.com/test",
            "invalid-url-format"
        };
        
        for (String testCase : testCases) {
            System.out.println("Input: " + testCase);
            System.out.println("Output: " + redirect_invalid_url(testCase));
            System.out.println();
        }
    }
}
