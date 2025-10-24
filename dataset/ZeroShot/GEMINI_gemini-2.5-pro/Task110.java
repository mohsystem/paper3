import java.net.MalformedURLException;
import java.net.URL;

public class Task110 {

    /**
     * Validates a URL using Java's built-in URL class.
     * This is a secure and robust method as it relies on the standard library's
     * well-tested parser. It also checks for a reasonable URL length.
     *
     * @param urlString The URL string to be validated.
     * @return true if the URL is valid, false otherwise.
     */
    public static boolean validateUrl(String urlString) {
        // 1. Check for null and unreasonable length to prevent resource exhaustion.
        if (urlString == null || urlString.length() > 2048) {
            return false;
        }

        try {
            // 2. The java.net.URL constructor parses the string and throws
            //    a MalformedURLException if it's not a valid URL.
            new URL(urlString);
            return true;
        } catch (MalformedURLException e) {
            // 3. The URL format is invalid.
            return false;
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://www.google.com",
            "http://example.com/path?name=value#fragment",
            "ftp://127.0.0.1:21/resource",
            "www.google.com", // Invalid: no scheme
            "http://invalid-.com", // Invalid: domain label ends with hyphen
            "garbage" // Invalid: not a URL
        };

        System.out.println("Java URL Validation:");
        for (String url : testUrls) {
            System.out.println("URL: \"" + url + "\" -> " + (validateUrl(url) ? "Valid" : "Invalid"));
        }
    }
}