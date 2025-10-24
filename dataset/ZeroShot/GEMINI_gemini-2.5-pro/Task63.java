import java.util.Base64;
import java.nio.charset.StandardCharsets;

public class Task63 {

    /**
     * Processes a raw request by Base64 decoding the data and preparing it for deserialization.
     *
     * @param rawData The Base64 encoded, UTF-8 string from the request.
     * @return The decoded data as a string, ready for safe deserialization. Returns null on error.
     */
    public static String processRequest(String rawData) {
        // Ensure input is not null or empty before processing.
        if (rawData == null || rawData.isEmpty()) {
            System.err.println("Error: Input data is null or empty.");
            return null;
        }

        try {
            // Step 1 & 2: Decode the raw_data using Base64, assuming UTF-8.
            // The getDecoder() method returns a standard Base64 decoder.
            byte[] decodedBytes = Base64.getDecoder().decode(rawData);
            String decodedString = new String(decodedBytes, StandardCharsets.UTF_8);

            // Step 3: Deserialize the decoded data.
            // SECURITY: For a single-file solution without external libraries, we stop here.
            // In a real-world application, this decodedString (expected to be JSON/XML)
            // MUST be deserialized using a SAFE parser.
            //
            // For JSON, use a library like Jackson or Gson.
            // Example with Jackson (requires the Jackson library):
            //   ObjectMapper mapper = new ObjectMapper();
            //   // Disable default typing to prevent remote code execution vulnerabilities.
            //   mapper.deactivateDefaultTyping();
            //   Map<String, Object> data = mapper.readValue(decodedString, new TypeReference<Map<String, Object>>(){});
            //
            // Never use insecure deserialization methods like Java's ObjectInputStream on
            // untrusted data, as it can lead to Remote Code Execution (RCE).

            // Step 4 & 5: Store the deserialized data in a variable and return it.
            // For this demonstration, we return the decoded string.
            return decodedString;

        } catch (IllegalArgumentException e) {
            // This exception is thrown if the input is not valid Base64.
            System.err.println("Error: Invalid Base64 input. " + e.getMessage());
            return null;
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            // Test Case 1: Valid JSON data
            "eyJ1c2VyIjogImFkbWluIiwgImlkIjogMTIzfQ==", // Decodes to: {"user": "admin", "id": 123}
            // Test Case 2: Another valid JSON
            "eyJrZXkiOiAidmFsdWUiLCAibnVtYmVyIjogNDJ9", // Decodes to: {"key": "value", "number": 42}
            // Test Case 3: Invalid Base64 string
            "invalid-base64-string",
            // Test Case 4: Valid Base64, but the content is not JSON
            "SGVsbG8gV29ybGQ=", // Decodes to: "Hello World"
            // Test Case 5: Empty string
            ""
        };

        int testNum = 1;
        for (String testCase : testCases) {
            System.out.println("--- Test Case " + testNum++ + " ---");
            System.out.println("Input: " + testCase);
            String result = processRequest(testCase);
            if (result != null) {
                System.out.println("Processed Data: " + result);
            } else {
                System.out.println("Processing failed.");
            }
            System.out.println();
        }
    }
}