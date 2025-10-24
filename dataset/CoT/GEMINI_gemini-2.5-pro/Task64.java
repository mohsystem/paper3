import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.MalformedURLException;
// Using the org.json library. Add it as a dependency.
// For Maven:
// <dependency>
//     <groupId>org.json</groupId>
//     <artifactId>json</artifactId>
//     <version>20231013</version>
// </dependency>
import org.json.JSONObject;
import org.json.JSONException;

public class Task64 {

    /**
     * Fetches content from a URL, parses it as JSON, and returns a JSONObject.
     *
     * @param urlString The URL to fetch the JSON from.
     * @return A JSONObject if successful, null otherwise.
     */
    public static JSONObject fetchJsonFromUrl(String urlString) {
        if (urlString == null || urlString.isEmpty()) {
            System.err.println("Error: URL string is null or empty.");
            return null;
        }

        HttpURLConnection connection = null;
        try {
            URL url = new URL(urlString);
            connection = (HttpURLConnection) url.openConnection();
            
            // Set timeouts to prevent the application from hanging.
            connection.setConnectTimeout(5000); // 5 seconds
            connection.setReadTimeout(5000);    // 5 seconds

            connection.setRequestMethod("GET");
            connection.setRequestProperty("Accept", "application/json");

            int responseCode = connection.getResponseCode();

            if (responseCode == HttpURLConnection.HTTP_OK) { // success
                StringBuilder response = new StringBuilder();
                try (BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream()))) {
                    String inputLine;
                    while ((inputLine = in.readLine()) != null) {
                        response.append(inputLine);
                    }
                }
                return new JSONObject(response.toString());
            } else {
                System.err.println("HTTP GET request failed with response code: " + responseCode);
                return null;
            }
        } catch (MalformedURLException e) {
            System.err.println("Error: Invalid URL provided. " + e.getMessage());
            return null;
        } catch (IOException e) {
            System.err.println("Error: Could not connect to the URL or read data. " + e.getMessage());
            return null;
        } catch (JSONException e) {
            System.err.println("Error: Failed to parse JSON response. " + e.getMessage());
            return null;
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    public static void main(String[] args) {
        // Command-line argument handling
        if (args.length > 0) {
            System.out.println("Fetching JSON from command-line argument: " + args[0]);
            JSONObject result = fetchJsonFromUrl(args[0]);
            if (result != null) {
                System.out.println("Successfully fetched and parsed JSON:\n" + result.toString(4));
            } else {
                System.out.println("Failed to fetch or parse JSON.");
            }
            System.out.println("\n--- Running built-in test cases ---\n");
        }


        String[] testUrls = {
            // 1. Valid JSON URL
            "https://jsonplaceholder.typicode.com/posts/1",
            // 2. URL pointing to non-JSON content (HTML)
            "http://example.com",
            // 3. URL that results in a 404 Not Found error
            "https://jsonplaceholder.typicode.com/posts/99999",
            // 4. Malformed URL
            "htp:/invalid-url",
            // 5. Non-existent domain
            "http://domain.that.does.not.exist"
        };

        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": " + testUrls[i] + " ---");
            JSONObject result = fetchJsonFromUrl(testUrls[i]);
            if (result != null) {
                // To keep output clean, we'll just print a success message and a snippet.
                System.out.println("Success! Fetched JSON object. Keys: " + result.keySet());
                // System.out.println(result.toString(4)); // Uncomment for full output
            } else {
                System.out.println("Failure! As expected or due to an error.");
            }
            System.out.println();
        }
    }
}