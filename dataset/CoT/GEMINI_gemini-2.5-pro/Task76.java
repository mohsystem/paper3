import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.time.Duration;

public class Task76 {

    /**
     * Makes an HTTPS call to a URL, retrieves the content, and extracts the page title.
     *
     * @param urlString The HTTPS URL to fetch.
     * @return The extracted page title, or an error message if it fails.
     */
    public static String getPageTitle(String urlString) {
        // 1. Input Validation: Ensure the URL is a valid HTTPS URL.
        try {
            URI uri = URI.create(urlString);
            if (!"https".equalsIgnoreCase(uri.getScheme())) {
                return "Error: URL must use HTTPS.";
            }
        } catch (IllegalArgumentException e) {
            return "Error: Invalid URL format.";
        }

        try {
            // 2. Use modern HttpClient with secure defaults (TLS, certificate validation)
            HttpClient client = HttpClient.newBuilder()
                    .followRedirects(HttpClient.Redirect.NORMAL)
                    .connectTimeout(Duration.ofSeconds(10))
                    .build();

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(urlString))
                    .timeout(Duration.ofSeconds(10))
                    .GET()
                    .build();

            // 3. Send request and get response
            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());

            // 4. Check for successful response
            if (response.statusCode() != 200) {
                return "Error: Received non-200 status code: " + response.statusCode();
            }

            String body = response.body();

            // 5. Secure and simple parsing: Avoid complex regex to prevent ReDoS.
            // Find the start and end of the title tag (case-insensitive search for tags)
            int titleStart = body.toLowerCase().indexOf("<title>");
            int titleEnd = body.toLowerCase().indexOf("</title>");

            if (titleStart != -1 && titleEnd != -1 && titleStart < titleEnd) {
                // Adjust titleStart to get the content inside the tag.
                titleStart += "<title>".length();
                return body.substring(titleStart, titleEnd).trim();
            } else {
                return "Error: Title tag not found.";
            }

        } catch (IOException | InterruptedException | IllegalArgumentException e) {
            // 6. Handle exceptions gracefully
            return "Error: Failed to fetch URL. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // Test cases
        String[] testUrls = {
            "https://www.google.com",
            "https://github.com",
            "https://www.microsoft.com/en-us/",
            "https://www.w3.org/",
            "https://example.com"
        };
        
        System.out.println("--- Running 5 Test Cases ---");
        for (String url : testUrls) {
            System.out.println("URL: " + url);
            String title = getPageTitle(url);
            System.out.println("Title: " + title);
            System.out.println("--------------------");
        }

        // Handling command line arguments
        if (args.length > 0) {
            System.out.println("\n--- Running with Command Line Argument ---");
            String urlFromArg = args[0];
            System.out.println("URL: " + urlFromArg);
            String title = getPageTitle(urlFromArg);
            System.out.println("Title: " + title);
            System.out.println("--------------------");
        } else {
            System.out.println("\nNote: You can also provide a URL as a command-line argument.");
        }
    }
}