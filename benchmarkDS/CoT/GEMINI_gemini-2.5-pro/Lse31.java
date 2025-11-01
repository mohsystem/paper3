import java.net.MalformedURLException;
import java.net.URL;

public class Lse31 {
    /**
     * Checks if a URL's domain is 'example.com' and returns a corresponding status message.
     * @param urlString The URL to check.
     * @return A string indicating redirection or a 404 error.
     */
    public static String redirect_url(String urlString) {
        if (urlString == null) {
            return "404 Not Found";
        }
        try {
            URL url = new URL(urlString);
            String domain = url.getHost();
            // Check if the domain is exactly "example.com"
            if ("example.com".equalsIgnoreCase(domain)) {
                return "Redirecting to " + urlString;
            } else {
                return "404 Not Found";
            }
        } catch (MalformedURLException e) {
            // A malformed URL cannot be from the target domain.
            return "404 Not Found";
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "http://example.com/home",
            "https://example.com/login?user=1",
            "http://www.google.com",
            "https://sub.example.com",
            "not-a-valid-url"
        };

        System.out.println("--- Running 5 Test Cases ---");
        for (String url : testUrls) {
            System.out.println("Input: " + url);
            System.out.println("Output: " + redirect_url(url));
            System.out.println("--------------------");
        }
    }
}