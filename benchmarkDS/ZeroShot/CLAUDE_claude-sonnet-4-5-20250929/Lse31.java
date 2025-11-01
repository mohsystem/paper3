
import java.net.URI;
import java.net.URISyntaxException;

public class Lse31 {
    
    public static String redirectUrl(String url) {
        try {
            URI parsedUrl = new URI(url);
            String networkLocation = parsedUrl.getHost();
            
            if (networkLocation != null && networkLocation.equals("example.com")) {
                return "Redirect to: " + url;
            } else {
                return "404 Error: Domain not allowed";
            }
        } catch (URISyntaxException e) {
            return "404 Error: Invalid URL";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "https://example.com/page1",
            "http://example.com/login",
            "https://notexample.com/page",
            "https://malicious.com/phishing",
            "https://sub.example.com/resource"
        };
        
        for (String testUrl : testCases) {
            System.out.println("URL: " + testUrl);
            System.out.println("Result: " + redirectUrl(testUrl));
            System.out.println();
        }
    }
}
