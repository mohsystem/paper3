import java.net.MalformedURLException;
import java.net.URL;

public class Lse33 {

    /**
     * Checks if the URL's domain is 'example.com' and returns an appropriate message.
     * This simulates the logic of a web route that performs a redirect.
     *
     * @param urlString The URL to check.
     * @return A string indicating redirection or an error.
     */
    public static String redirect_invalid_url(String urlString) {
        if (urlString == null || urlString.isEmpty()) {
            return "Error: URL is empty.";
        }

        URL url;
        try {
            // Use java.net.URL for robust and secure parsing.
            // This correctly handles various URL formats and prevents parsing bypasses.
            url = new URL(urlString);
        } catch (MalformedURLException e) {
            return "Error: Malformed URL provided.";
        }

        // getHost() correctly extracts the hostname, ignoring user info, ports, etc.
        String host = url.getHost();

        // Perform a strict, case-insensitive comparison against the allowed domain.
        if ("example.com".equalsIgnoreCase(host)) {
            // In a real web application, this would be an HTTP 302/307 redirect.
            return "Redirecting to " + urlString;
        } else {
            // This prevents open redirect vulnerabilities by rejecting other domains.
            return "Error: Domain '" + host + "' is not allowed.";
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            // 1. Valid URL
            "https://example.com/some/path",
            // 2. Another valid URL with a different scheme
            "http://example.com",
            // 3. Invalid domain
            "https://www.google.com",
            // 4. Open redirect attempt via query parameter (should be blocked)
            "https://evil.com?q=example.com",
            // 5. Open redirect attempt via user info (should be blocked)
            "https://example.com@evil.com/login"
        };

        System.out.println("Running Java Test Cases:");
        for (String url : testUrls) {
            System.out.println("Input: " + url);
            System.out.println("Output: " + redirect_invalid_url(url));
            System.out.println("--------------------");
        }
    }
}