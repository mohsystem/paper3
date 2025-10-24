import java.util.Base64;
import java.nio.charset.StandardCharsets;

public class Task63 {

    /**
     * Processes a request by decoding a Base64 encoded UTF-8 string.
     * In a real-world application, the returned string (assumed to be JSON, XML, etc.)
     * would then be parsed by a dedicated and secure library (e.g., Jackson, Gson).
     * This function performs the decoding step and returns the raw deserialized string.
     *
     * @param base64Utf8Data The Base64 encoded UTF-8 string.
     * @return The decoded string, or null on failure.
     */
    public static String processRequest(String base64Utf8Data) {
        if (base64Utf8Data == null) {
            System.err.println("Error: Input data cannot be null.");
            return null;
        }
        try {
            // 1. Retrieve raw_data from the request.
            // 2. Decode the raw_data using Base64.
            byte[] decodedBytes = Base64.getDecoder().decode(base64Utf8Data);

            // 3. Deserialize the decoded data by converting bytes to a UTF-8 string.
            // This string is the "deserialized" data (e.g., JSON, XML).
            // 4. Store in a variable and return.
            return new String(decodedBytes, StandardCharsets.UTF_8);

        } catch (IllegalArgumentException e) {
            // This catches errors from Base64.getDecoder().decode() for malformed Base64.
            System.err.println("Error: Invalid Base64 data provided.");
            return null;
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            // Test Case 1: Simple valid JSON
            "eyJpZCI6IDEyMywgInVzZXIiOiAidGVzdCJ9", // {"id": 123, "user": "test"}
            // Test Case 2: More complex JSON
            "eyJuYW1lIjogIkJvYiIsICJhY3RpdmUiOiB0cnVlLCAicm9sZXMiOiBbImFkbWluIiwgImVkaXRvciJdfQ==", // {"name": "Bob", "active": true, "roles": ["admin", "editor"]}
            // Test Case 3: Empty JSON object
            "e30=", // {}
            // Test Case 4: Invalid Base64 string
            "invalid-base64@#$",
            // Test Case 5: Valid Base64, but not valid JSON (the function just decodes it)
            "VGhpcyBpcyBub3QgSlNPTg==" // "This is not JSON"
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input: " + testCases[i]);
            String result = processRequest(testCases[i]);
            if (result != null) {
                System.out.println("Output: " + result);
            } else {
                System.out.println("Output: Processing failed.");
            }
            System.out.println();
        }
    }
}