import java.util.function.Function;

public class Task62 {

    /**
     * Identifies the root element type of a JSON-like string.
     * This is a non-validating heuristic. It checks the first and last
     * non-whitespace characters and attempts to parse the string as a known
     * literal or a number. It does not perform a full, validating JSON parse.
     *
     * @param jsonString The string to analyze.
     * @return A string representing the root element type ("object", "array", "string",
     *         "number", "boolean", "null") or "invalid" if it cannot be determined.
     */
    public static String getRootElementType(String jsonString) {
        if (jsonString == null) {
            return "invalid";
        }

        String trimmed = jsonString.trim();

        if (trimmed.isEmpty()) {
            return "invalid";
        }

        char firstChar = trimmed.charAt(0);
        // Check length > 1 for string, array, object to avoid index out of bounds on single char strings
        if (trimmed.length() > 1) {
            char lastChar = trimmed.charAt(trimmed.length() - 1);
            if (firstChar == '{' && lastChar == '}') {
                return "object";
            }
            if (firstChar == '[' && lastChar == ']') {
                return "array";
            }
            if (firstChar == '"' && lastChar == '"') {
                return "string";
            }
        }

        if (trimmed.equals("true") || trimmed.equals("false")) {
            return "boolean";
        }

        if (trimmed.equals("null")) {
            return "null";
        }

        // Check for number by attempting to parse as Double
        try {
            Double.parseDouble(trimmed);
            return "number";
        } catch (NumberFormatException e) {
            // Not a valid number, fall through to return "invalid"
        }
        
        return "invalid";
    }

    public static void main(String[] args) {
        String[] testCases = {
            "  { \"key\": \"value\" }  ",
            " [1, 2, 3] ",
            "\"a string\"",
            "123.45",
            "true"
        };

        String[] expected = {
            "object",
            "array",
            "string",
            "number",
            "boolean"
        };
        
        System.out.println("Running Java tests...");
        for (int i = 0; i < testCases.length; i++) {
            String result = getRootElementType(testCases[i]);
            System.out.printf("Test Case %d: -> %s (Expected: %s) -> %s\n",
                i + 1, result, expected[i], result.equals(expected[i]) ? "PASS" : "FAIL");
        }
    }
}