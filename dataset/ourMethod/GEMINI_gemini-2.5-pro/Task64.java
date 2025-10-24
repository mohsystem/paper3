/*
 * This code requires the 'org.json' library.
 * To use with a build tool like Maven, add the following to your pom.xml:
 * <dependency>
 *     <groupId>org.json</groupId>
 *     <artifactId>json</artifactId>
 *     <version>20231013</version>
 * </dependency>
 *
 * To compile and run from the command line:
 * 1. Download the jar, e.g., from Maven Central (search for 'org.json json').
 * 2. Compile: javac -cp path/to/json-20231013.jar Task64.java
 * 3. Run:     java -cp .:path/to/json-20231013.jar Task64 <optional_url>
 */
import org.json.JSONObject;
import org.json.JSONException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.io.IOException;
import java.time.Duration;

public class Task64 {

    /**
     * Fetches a JSON file from a URL, parses it, and returns a JSONObject.
     * @param urlString The URL of the JSON resource.
     * @return A JSONObject representing the parsed JSON.
     * @throws Exception if the URL is invalid, the request fails, or parsing fails.
     */
    public static JSONObject fetchAndParseJson(String urlString) throws Exception {
        if (urlString == null || urlString.trim().isEmpty()) {
            throw new IllegalArgumentException("URL cannot be null or empty.");
        }

        URI uri;
        try {
            uri = new URI(urlString);
            String scheme = uri.getScheme();
            if (!"https".equalsIgnoreCase(scheme)) { // Enforce HTTPS for security
                throw new IllegalArgumentException("Invalid URL scheme. Only HTTPS is supported.");
            }
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("Invalid URL syntax: " + e.getMessage(), e);
        }

        HttpClient client = HttpClient.newBuilder()
                .version(HttpClient.Version.HTTP_2)
                .followRedirects(HttpClient.Redirect.NEVER) // Avoid open redirect vulnerabilities
                .connectTimeout(Duration.ofSeconds(10))
                .build();

        HttpRequest request = HttpRequest.newBuilder()
                .uri(uri)
                .timeout(Duration.ofSeconds(10))
                .header("Accept", "application/json")
                .GET()
                .build();

        try {
            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());

            if (response.statusCode() != 200) {
                throw new IOException("HTTP request failed with status code: " + response.statusCode());
            }

            String responseBody = response.body();
            return new JSONObject(responseBody);

        } catch (IOException | InterruptedException e) {
            throw new Exception("Failed to fetch data from URL: " + e.getMessage(), e);
        } catch (JSONException e) {
            throw new Exception("Failed to parse JSON response: " + e.getMessage(), e);
        }
    }

    public static void main(String[] args) {
        // Use command line argument if provided
        if (args.length > 0) {
            System.out.println("--- Testing with provided URL: " + args[0] + " ---");
            try {
                JSONObject result = fetchAndParseJson(args[0]);
                System.out.println("Success! Fetched JSON object:");
                System.out.println(result.toString(4));
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
            }
            System.out.println("\n--- Running built-in test cases ---\n");
        }

        // 5 Test cases
        String[] testUrls = {
            // 1. Valid JSON API endpoint
            "https://jsonplaceholder.typicode.com/posts/1",
            // 2. URL leading to non-JSON content (HTML)
            "https://www.google.com",
            // 3. URL that results in a 404 Not Found error
            "https://jsonplaceholder.typicode.com/posts/99999999",
            // 4. Malformed URL
            "htps:/invalid-url",
            // 5. URL with a non-HTTPS scheme (will be rejected)
            "http://jsonplaceholder.typicode.com/posts/1"
        };

        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": " + testUrls[i] + " ---");
            try {
                JSONObject result = fetchAndParseJson(testUrls[i]);
                System.out.println("Success! Fetched JSON object:");
                System.out.println(result.toString(4));
            } catch (Exception e) {
                // Print a cleaner message for known error types
                Throwable cause = e.getCause();
                if (cause instanceof java.net.UnknownHostException) {
                    System.err.println("Error: Unknown host - check domain name and network connection.");
                } else {
                    System.err.println("Error: " + e.getMessage());
                }
            }
            System.out.println();
        }
    }
}