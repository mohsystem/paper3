// Note: This code requires the 'org.json' library.
// You can add it to your project via Maven/Gradle. For example, for Maven:
// <dependency>
//     <groupId>org.json</groupId>
//     <artifactId>json</artifactId>
//     <version>20231013</version>
// </dependency>
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONTokener;

public class Task64 {

    /**
     * Fetches content from a URL, parses it as JSON, and returns the resulting object.
     * The returned value can be cast to org.json.JSONObject or org.json.JSONArray.
     *
     * @param urlString The URL to fetch JSON from.
     * @return An Object representing the parsed JSON (JSONObject or JSONArray).
     * @throws Exception if there's an error in fetching or parsing.
     */
    public static Object readJsonFromUrl(String urlString) throws Exception {
        URL url = new URL(urlString);
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();
        connection.setRequestMethod("GET");
        connection.setConnectTimeout(5000);
        connection.setReadTimeout(5000);

        int responseCode = connection.getResponseCode();
        if (responseCode >= 200 && responseCode < 300) {
            BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            StringBuilder response = new StringBuilder();
            String inputLine;
            while ((inputLine = in.readLine()) != null) {
                response.append(inputLine);
            }
            in.close();

            JSONTokener tokener = new JSONTokener(response.toString());
            return tokener.nextValue();
        } else {
            throw new RuntimeException("HTTP GET request failed with error code: " + responseCode);
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://jsonplaceholder.typicode.com/todos/1",
            "https://jsonplaceholder.typicode.com/posts/1",
            "https://jsonplaceholder.typicode.com/users/1",
            "https://jsonplaceholder.typicode.com/posts", // This is a JSON array
            "https://jsonplaceholder.typicode.com/nonexistent" // This will cause an error
        };

        for (String url : testUrls) {
            System.out.println("Fetching from: " + url);
            try {
                Object json = readJsonFromUrl(url);
                if (json instanceof JSONObject) {
                    System.out.println(((JSONObject) json).toString(4));
                } else if (json instanceof JSONArray) {
                    System.out.println(((JSONArray) json).toString(4));
                }
            } catch (Exception e) {
                System.err.println("Error: " + e.getMessage());
            }
            System.out.println("----------------------------------------");
        }
    }
}