import java.net.URL;

public class Task110 {

    /**
     * Validates a URL string using Java's built-in URL class.
     * It checks for protocol and proper formatting.
     * @param urlString The URL string to validate.
     * @return true if the URL is valid, false otherwise.
     */
    public static boolean isValidURL(String urlString) {
        if (urlString == null) {
            return false;
        }
        try {
            // The URL constructor will throw a MalformedURLException if the URL is invalid.
            // toURI() is called for a stricter syntax check.
            new URL(urlString).toURI();
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://www.google.com",
            "http://example.org/path?name=value",
            "ftp://example.com",
            "https://example..com",
            "http://invalid",
            "just a string"
        };

        System.out.println("--- Java URL Validation ---");
        for (String url : testUrls) {
            System.out.println("URL: \"" + url + "\" is " + (isValidURL(url) ? "valid" : "invalid"));
        }
    }
}