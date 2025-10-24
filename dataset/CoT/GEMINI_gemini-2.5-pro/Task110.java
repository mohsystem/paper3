import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task110 {

    /**
     * Validates a URL using a regular expression.
     * This regex checks for http, https, or ftp schemes, a valid domain name,
     * and an optional path.
     *
     * @param url The string to validate.
     * @return true if the URL is valid, false otherwise.
     */
    public static boolean isValidURL(String url) {
        // Handle null or empty input
        if (url == null || url.trim().isEmpty()) {
            return false;
        }

        // Regex for a standard URL (http, https, ftp)
        // ^(https?|ftp)://    - Scheme (http, https, ftp)
        // [a-zA-Z0-9.-]+      - Domain name characters
        // \.[a-zA-Z]{2,}      - Top-level domain (at least 2 letters)
        // (?:/[^\s]*)?$       - Optional path (any non-whitespace characters)
        String urlRegex = "^(https?|ftp)://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(?:/[^\\s]*)?$";

        Pattern pattern = Pattern.compile(urlRegex);
        Matcher matcher = pattern.matcher(url);
        return matcher.matches();
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://www.example.com",
            "http://example.co.uk/path/to/page?query=123",
            "ftp://ftp.example.com/resource",
            "htp://invalid-scheme.com", // Invalid scheme
            "https://.invalid-domain.com", // Invalid domain
            "https://domain-without-tld", // Invalid TLD
            "Just a string", // Not a URL
            "" // Empty string
        };

        System.out.println("Running URL validation test cases in Java:");
        for (String url : testUrls) {
            System.out.println("URL: \"" + url + "\" -> " + (isValidURL(url) ? "Valid" : "Invalid"));
        }
    }
}