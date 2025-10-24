import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;

public class Task110 {

    /**
     * Validates if a given string is a well-formed URL.
     *
     * @param urlString The string to validate.
     * @return true if the string is a valid URL, false otherwise.
     */
    public static boolean isValidURL(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            return false;
        }
        try {
            // Use Java's built-in URL and URI classes for robust validation.
            // Creating a URL object checks for basic syntax.
            // Converting it to a URI provides stricter syntax checking.
            new URL(urlString).toURI();
            return true;
        } catch (MalformedURLException | URISyntaxException e) {
            return false;
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://www.google.com",
            "http://example.com/path?query=1",
            "ftp://ftp.is.co.za/rfc/rfc1808.txt",
            "www.invalid.com",
            "https://",
            "http://exa mple.com"
        };

        System.out.println("Running URL validation tests:");
        for (String url : testUrls) {
            System.out.printf("URL: \"%s\" -> Valid: %b%n", url, isValidURL(url));
        }
    }
}