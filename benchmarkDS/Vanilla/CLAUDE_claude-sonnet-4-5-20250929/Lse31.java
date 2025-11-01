
import java.net.URI;
import java.net.URISyntaxException;

public class Lse31 {
    public static String redirect_url(String url) {
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
        System.out.println(redirect_url("http://example.com/page"));
        System.out.println(redirect_url("https://example.com/admin"));
        System.out.println(redirect_url("http://notexample.com/page"));
        System.out.println(redirect_url("https://evil.com/phishing"));
        System.out.println(redirect_url("http://example.com:8080/secure"));
    }
}
