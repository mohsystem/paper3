/**
 * This code simulates the backend logic of a web route that processes a JSON payload.
 * It is designed to be run as a single file.
 *
 * Dependency: org.json library. To compile and run, you need this library in your classpath.
 * If using Maven, add this to your pom.xml:
 * <dependency>
 *     <groupId>org.json</groupId>
 *     <artifactId>json</artifactId>
 *     <version>20231013</version>
 * </dependency>
 */
import org.json.JSONException;
import org.json.JSONObject;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse82 {

    /**
     * Parses a URL query string into a map of key-value pairs.
     * This is a simplified parser for demonstration purposes.
     * @param query The URL query string.
     * @return A map of parameters.
     */
    private static Map<String, String> parseQuery(String query) {
        Map<String, String> params = new HashMap<>();
        if (query == null || query.isEmpty()) {
            return params;
        }
        for (String param : query.split("&")) {
            String[] pair = param.split("=", 2);
            if (pair.length > 1) {
                params.put(pair[0], pair[1]);
            } else if (pair.length == 1 && !pair[0].isEmpty()) {
                params.put(pair[0], "");
            }
        }
        return params;
    }

    /**
     * Simulates processing a request to an /api endpoint. It extracts a 'payload'
     * parameter from the query string, parses it as JSON, and returns the value 
     * associated with the 'name' key.
     *
     * @param queryString The query string from the URL (e.g., "payload=%7B...%7D").
     * @return The value of the 'name' key if found, otherwise an error message.
     */
    public static String processApiRequest(String queryString) {
        Map<String, String> params = parseQuery(queryString);

        if (!params.containsKey("payload")) {
            return "Error: 'payload' parameter not found.";
        }

        String payloadEncoded = params.get("payload");
        if (payloadEncoded == null || payloadEncoded.isEmpty()) {
            return "Error: 'payload' parameter is empty.";
        }
        
        try {
            String payloadDecoded = URLDecoder.decode(payloadEncoded, StandardCharsets.UTF_8.name());
            
            JSONObject json = new JSONObject(payloadDecoded);

            if (!json.has("name")) {
                return "Error: 'name' key not found in payload.";
            }

            // Securely check if the 'name' value is a String before casting
            Object nameValue = json.get("name");
            if (!(nameValue instanceof String)) {
                 return "Error: 'name' value is not a string.";
            }

            return (String) nameValue;

        } catch (UnsupportedEncodingException e) {
            // This is highly unlikely with a standard charset like UTF-8
            return "Error: Unsupported encoding specified.";
        } catch (JSONException e) {
            return "Error: Malformed JSON in payload.";
        } catch (Exception e) {
            // Catch-all for any other unexpected issues
            return "Error: An unexpected error occurred.";
        }
    }

    /**
     * Main method to run test cases for the processApiRequest function.
     */
    public static void main(String[] args) {
        String[] testCases = {
            // 1. Valid case with a 'name' key
            "payload=%7B%22name%22%3A%22Alice%22%2C%20%22id%22%3A123%7D",
            // 2. Valid JSON but missing the 'name' key
            "payload=%7B%22id%22%3A456%7D",
            // 3. Malformed JSON (extra comma)
            "payload=%7B%22name%22%3A%22Bob%22%2C%7D",
            // 4. Query string without a 'payload' parameter
            "other_param=abc&another=123",
            // 5. 'name' key exists but its value is not a string
            "payload=%7B%22name%22%3A12345%7D"
        };

        System.out.println("Running test cases for Lse82...");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Input: " + testCases[i]);
            String result = processApiRequest(testCases[i]);
            System.out.println("Output: " + result);
        }
    }
}