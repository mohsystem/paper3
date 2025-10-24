public class Task62 {

    /**
     * Parses the provided string to identify the type of the root JSON element.
     *
     * @param jsonString The string representing a JSON document.
     * @return A string indicating the root element's type ("Object", "Array", "String",
     *         "Number", "Boolean", "Null", or "Invalid").
     */
    public static String getRootElementType(String jsonString) {
        if (jsonString == null || jsonString.isEmpty()) {
            return "Invalid";
        }

        String trimmed = jsonString.trim();

        if (trimmed.isEmpty()) {
            return "Invalid";
        }

        char firstChar = trimmed.charAt(0);
        char lastChar = trimmed.charAt(trimmed.length() - 1);

        if (firstChar == '{' && lastChar == '}') {
            return "Object";
        }
        if (firstChar == '[' && lastChar == ']') {
            return "Array";
        }
        if (firstChar == '"' && lastChar == '"') {
            return "String";
        }
        if (trimmed.equals("true") || trimmed.equals("false")) {
            return "Boolean";
        }
        if (trimmed.equals("null")) {
            return "Null";
        }

        // Try to parse as a number
        try {
            Double.parseDouble(trimmed);
            return "Number";
        } catch (NumberFormatException e) {
            // Not a valid number
        }

        return "Invalid";
    }

    public static void main(String[] args) {
        String[] testCases = {
            "  { \"name\": \"John\", \"age\": 30 }  ", // Test case 1: Object
            "[1, \"apple\", true, null]",              // Test case 2: Array
            "\"This is a root string.\"",              // Test case 3: String
            "  -123.45e6  ",                          // Test case 4: Number
            "null"                                     // Test case 5: Null
        };

        System.out.println("Running Java Test Cases:");
        for (int i = 0; i < testCases.length; i++) {
            String json = testCases[i];
            String result = getRootElementType(json);
            System.out.println("Test Case " + (i + 1) + ": \"" + json + "\" -> " + result);
        }
    }
}