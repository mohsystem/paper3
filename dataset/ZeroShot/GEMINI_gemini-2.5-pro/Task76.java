import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpTimeoutException;
import java.time.Duration;

public class Task76 {

    /**
     * Fetches an HTTPS URL and extracts the content of the <title> tag.
     *
     * @param urlString The HTTPS URL to fetch.
     * @return The extracted page title, or an error message.
     */
    public static String extractPageTitle(String urlString) {
        // 1. Input Validation: Ensure URL is not null and uses HTTPS.
        if (urlString == null || !urlString.toLowerCase().startsWith("https://")) {
            return "Error: Invalid or insecure URL provided. Please use HTTPS.";
        }

        try {
            // 2. Setup HttpClient: Use modern, secure defaults.
            HttpClient client = HttpClient.newBuilder()
                    .version(HttpClient.Version.HTTP_2)
                    .followRedirects(HttpClient.Redirect.NORMAL)
                    .connectTimeout(Duration.ofSeconds(10)) // Connection timeout
                    .build();

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(urlString))
                    .timeout(Duration.ofSeconds(10)) // Request timeout
                    .header("User-Agent", "Java HttpClient Bot") // Set a user agent
                    .GET()
                    .build();

            // 3. Send Request and Get Response
            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());

            // 4. Handle HTTP Errors
            if (response.statusCode() != 200) {
                return "Error: Received non-200 status code: " + response.statusCode();
            }

            String body = response.body();

            // 5. Extract Title using safe string searching (not complex regex)
            int titleStart = body.toLowerCase().indexOf("<title>");
            if (titleStart == -1) {
                return "Error: Title tag not found.";
            }
            // Move index to the end of the opening tag
            titleStart += "<title>".length();

            int titleEnd = body.toLowerCase().indexOf("</title>", titleStart);
            if (titleEnd == -1) {
                return "Error: Closing title tag not found.";
            }

            // Extract, trim, and return the title
            return body.substring(titleStart, titleEnd).trim();

        } catch (IllegalArgumentException e) {
            return "Error: Invalid URL format - " + e.getMessage();
        } catch (HttpTimeoutException e) {
            return "Error: Request timed out - " + e.getMessage();
        } catch (Exception e) { // Catches IOException, InterruptedException etc.
            return "Error: An exception occurred - " + e.getClass().getSimpleName() + ": " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://www.google.com",
            "https://github.com",
            "https://github.com/nonexistent/repo", // 404 test
            "https://thissitedoesnotexist12345.com", // DNS error
            "http://www.google.com" // Insecure URL test
        };

        for (String url : testUrls) {
            System.out.println("Testing URL: " + url);
            String title = extractPageTitle(url);
            System.out.println("Page Title: " + title);
            System.out.println("--------------------");
        }
    }
}