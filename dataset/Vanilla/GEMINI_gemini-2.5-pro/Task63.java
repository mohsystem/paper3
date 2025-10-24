import java.util.Base64;
import java.nio.charset.StandardCharsets;

public class Task63 {

    /**
     * Processes a raw request by Base64 decoding and deserializing the data.
     *
     * @param rawData The Base64 encoded, UTF-8 string data.
     * @return The deserialized data as a String (in this case, a JSON string).
     *         For full deserialization into a Java object, a library like Gson or Jackson would be used.
     */
    public static String processRequest(String rawData) {
        // Step 1: Retrieve the raw_data. (Passed as parameter)
        // Step 2: Decode the raw_data using Base64. It is assumed the original data was UTF-8.
        byte[] decodedBytes = Base64.getDecoder().decode(rawData);

        // Step 3: Deserialize the decoded data into its original format (String).
        String deserializedData = new String(decodedBytes, StandardCharsets.UTF_8);

        // Step 4 & 5: Store the deserialized data in a variable and return it.
        return deserializedData;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "eyJpZCI6IDEyMywgIm5hbWUiOiAidGVzdF91c2VyIiwgImFjdGl2ZSI6IHRydWV9", // {"id": 123, "name": "test_user", "active": true}
            "eyJtZXNzYWdlIjogIkhlbGxvLCBXb3JsZCEifQ==", // {"message": "Hello, World!"}
            "eyJkYXRhIjogWzEsIDIsIDNdLCAic3RhdHVzIjogIm9rIn0=", // {"data": [1, 2, 3], "status": "ok"}
            "eyJlcnJvciI6ICJOb3QgRm91bmQiLCAiY29kZSI6IDQwNH0=", // {"error": "Not Found", "code": 404}
            "e30=" // {}
        };

        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            String rawData = testCases[i];
            System.out.println("Input (Base64): " + rawData);
            String result = processRequest(rawData);
            System.out.println("Output (Deserialized): " + result);
            System.out.println();
        }
    }
}