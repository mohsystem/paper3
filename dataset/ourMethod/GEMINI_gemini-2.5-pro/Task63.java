import java.util.Base64;
import java.nio.charset.StandardCharsets;
// Note: This code requires the org.json library (e.g., from Maven: org.json:json:20231013).
// To compile and run this file as a single unit, you must have the library JAR in your classpath.
// E.g., javac -cp .;json-20231013.jar Task63.java
//       java -cp .;json-20231013.jar Task63
import org.json.JSONObject;
import org.json.JSONException;

public class Task63 {

    /**
     * Processes a request by decoding and deserializing the raw data.
     *
     * @param rawData A UTF-8 string containing Base64 encoded JSON data.
     * @return A JSONObject if processing is successful, null otherwise.
     */
    public static JSONObject processRequest(String rawData) {
        if (rawData == null) {
            return null;
        }
        try {
            // Retrieve the raw_data and decode it using Base64.
            // The input string is assumed to be UTF-8 encoded characters representing Base64 data.
            byte[] decodedBytes = Base64.getDecoder().decode(rawData.getBytes(StandardCharsets.UTF_8));
            
            // Convert decoded bytes to a string, assuming the original data was UTF-8.
            String jsonString = new String(decodedBytes, StandardCharsets.UTF_8);

            // Deserialize the JSON string into a JSONObject.
            JSONObject data = new JSONObject(jsonString);

            // Return the deserialized data.
            return data;
        } catch (IllegalArgumentException | JSONException e) {
            // This catches errors from Base64 decoding (IllegalArgumentException)
            // or JSON parsing (JSONException).
            return null;
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            // Test Case 1: Valid input
            "eyJ1c2VyIjogInRlc3QiLCAiaWQiOiAxMjMsICJhY3RpdmUiOiB0cnVlfQ==",
            // Test Case 2: Another valid input
            "eyJoZWxsbyI6ICJ3b3JsZCJ9",
            // Test Case 3: Invalid Base64
            "this-is-not-base64",
            // Test Case 4: Valid Base64, but not JSON
            "aGVsbG8gd29ybGQ=", // "hello world"
            // Test Case 5: Empty input
            ""
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: \"" + testCases[i] + "\"");
            JSONObject result = processRequest(testCases[i]);
            if (result != null) {
                // Use an indent factor of 2 for pretty printing
                System.out.println("Output: " + result.toString(2));
            } else {
                System.out.println("Output: Processing failed (null)");
            }
            System.out.println("--------------------");
        }
    }
}