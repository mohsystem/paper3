import java.net.URI;
import java.net.URISyntaxException;

public class Lse31 {
    /**
     * Checks if a URL's host is 'example.com' and returns a redirect message.
     * Otherwise, returns a 404 error message. This implementation is secure
     * against open redirect vulnerabilities by strictly checking the hostname.
     *
     * @param url The URL string to validate.
     * @return A string indicating redirection or a 404 error.
     */
    public static String redirect_url(String url) {
        if (url == null) {
            return "404 Not Found - Null URL";
        }
        try {
            URI uri = new URI(url);
            String host = uri.getHost();

            // The host must exist and be exactly 'example.com' (case-insensitive).
            if (host != null && host.equalsIgnoreCase("example.com")) {
                // In a real web application, this would trigger an HTTP 301/302 response.
                // For this example, we return a success message with the original URL.
                return "Redirecting to: " + url;
            } else {
                return "404 Not Found";
            }
        } catch (URISyntaxException e) {
            // The provided string is not a valid URI.
            return "404 Not Found - Invalid URL";
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://example.com/path/to/resource",
            "http://www.google.com",
            "https://evil-example.com",
            "https://example.com.hacker.com/login",
            "ftp://example.com:21/files"
        };

        System.out.println("Running Java Test Cases:");
        for (String url : testUrls) {
            System.out.println("Input: " + url);
            System.out.println("Output: " + redirect_url(url));
            System.out.println("--------------------");
        }
    }
}