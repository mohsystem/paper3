/*
 * Requires Google's Gson library.
 * If using a build tool like Maven, add this to your pom.xml:
 * <dependency>
 *     <groupId>com.google.code.gson</groupId>
 *     <artifactId>gson</artifactId>
 *     <version>2.10.1</version>
 * </dependency>
 * If compiling manually, download the Gson jar and include it in your classpath:
 * javac -cp gson-2.10.1.jar Task62.java
 * java -cp .:gson-2.10.1.jar Task62
 */
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;
import com.google.gson.JsonSyntaxException;
import com.google.gson.JsonPrimitive;

public class Task62 {

    /**
     * Parses a JSON string and returns its root element.
     * This method uses the robust Gson library to safely parse the JSON,
     * protecting against malformed input and potential parsing vulnerabilities.
     *
     * @param jsonString The JSON string to parse.
     * @return The root JsonElement if parsing is successful, null otherwise.
     */
    public static JsonElement getJsonRoot(String jsonString) {
        if (jsonString == null || jsonString.trim().isEmpty()) {
            System.err.println("Error: Input JSON string is null or empty.");
            return null;
        }
        try {
            // JsonParser.parseString is designed to handle various forms of
            // JSON safely and efficiently.
            return JsonParser.parseString(jsonString);
        } catch (JsonSyntaxException e) {
            // Securely handle parsing errors without exposing sensitive internal details.
            // Log the specific error for debugging but return a simple failure status.
            System.err.println("Error parsing JSON string: Invalid syntax.");
            return null;
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            // 1. Valid JSON object
            "{\"name\": \"John Doe\", \"age\": 30, \"isStudent\": false, \"courses\": [\"Math\", \"Science\"]}",
            // 2. Valid JSON array
            "[1, \"test\", true, null, {\"key\": \"value\"}]",
            // 3. Valid JSON primitive (string)
            "\"Hello, Secure World!\"",
            // 4. Valid JSON primitive (number)
            "123.456",
            // 5. Invalid JSON string (malformed)
            "{\"name\": \"Jane Doe\", \"age\":}"
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input: " + testCases[i]);
            JsonElement rootElement = getJsonRoot(testCases[i]);

            if (rootElement != null) {
                System.out.println("Root Element Type: " + getElementType(rootElement));
                System.out.println("Parsed Output: " + rootElement.toString());
            } else {
                System.out.println("Failed to parse JSON.");
            }
            System.out.println();
        }
    }
    
    /**
     * Helper method to determine the type of a JsonElement for display purposes.
     */
    private static String getElementType(JsonElement element) {
        if (element.isJsonObject()) return "Object";
        if (element.isJsonArray()) return "Array";
        if (element.isJsonPrimitive()) {
             JsonPrimitive primitive = element.getAsJsonPrimitive();
             if (primitive.isString()) return "Primitive (String)";
             if (primitive.isNumber()) return "Primitive (Number)";
             if (primitive.isBoolean()) return "Primitive (Boolean)";
        }
        if (element.isJsonNull()) return "Null";
        return "Unknown";
    }
}