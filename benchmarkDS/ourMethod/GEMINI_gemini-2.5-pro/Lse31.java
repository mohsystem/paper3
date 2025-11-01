import java.net.URI;
import java.net.URISyntaxException;

public class Lse31 {

    /**
     * Checks the domain of a URL and returns a redirect or 404 message.
     *
     * @param urlString The URL to check.
     * @return A string indicating redirection or a 404 error.
     */
    public static String redirectUrl(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            return "404 Not Found";
        }

        try {
            URI uri = new URI(urlString);
            String domain = uri.getHost();

            // The host can be null for non-absolute URIs like "path/to/file"
            if (domain != null && "example.com".equals(domain)) {
                // Sanitize urlString for output to prevent potential log injection if this were logged,
                // though here it's just for display. A simple check is to remove newlines.
                String safeUrl = urlString.replace('\n', '_').replace('\r', '_');
                return "Redirecting to " + safeUrl;
            } else {
                return "404 Not Found";
            }
        } catch (URISyntaxException e) {
            // Malformed URLs are considered "not found"
            return "404 Not Found";
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "https://example.com/some/path",  // Case 1: Match
            "http://google.com/search",       // Case 2: No Match
            "invalid-url-format",             // Case 3: Invalid format
            "https://another-site.net",       // Case 4: Another No Match
            "http://example.com"              // Case 5: Match without path
        };

        System.out.println("--- Running 5 Test Cases ---");
        for (String testUrl : testCases) {
             System.out.println("Input: " + testUrl);
             System.out.println("Output: " + redirectUrl(testUrl));
             System.out.println("--------------------");
        }
    }
}