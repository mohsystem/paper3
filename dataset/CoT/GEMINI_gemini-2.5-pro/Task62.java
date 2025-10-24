import java.util.Objects;

public class Task62 {

    /**
     * Parses the JSON string to identify the root element's type.
     * This implementation performs a basic structural check by finding the first and
     * last non-whitespace characters to determine if the document is enclosed
     * in braces `{}` (Object) or brackets `[]` (Array).
     *
     * @param jsonString The string representation of the JSON document.
     * @return "Object", "Array", or "Invalid" based on the root element.
     */
    public static String findRootElement(String jsonString) {
        if (jsonString == null || jsonString.isEmpty()) {
            return "Invalid";
        }

        int first = -1;
        int last = -1;

        // Find the first non-whitespace character
        for (int i = 0; i < jsonString.length(); i++) {
            if (!Character.isWhitespace(jsonString.charAt(i))) {
                first = i;
                break;
            }
        }

        // If no non-whitespace character is found, the string is empty or all whitespace
        if (first == -1) {
            return "Invalid";
        }

        // Find the last non-whitespace character
        for (int i = jsonString.length() - 1; i >= 0; i--) {
            if (!Character.isWhitespace(jsonString.charAt(i))) {
                last = i;
                break;
            }
        }

        char firstChar = jsonString.charAt(first);
        char lastChar = jsonString.charAt(last);

        if (firstChar == '{' && lastChar == '}') {
            return "Object";
        } else if (firstChar == '[' && lastChar == ']') {
            return "Array";
        } else {
            return "Invalid";
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "  { \"name\": \"John\", \"age\": 30 }  ",
            "[ \"apple\", \"banana\", \"cherry\" ]",
            "   ",
            "this is not json",
            "{ \"incomplete\": \"json\""
        };

        System.out.println("Java Test Cases:");
        for (String testCase : testCases) {
            System.out.println("Input: \"" + testCase + "\" -> Output: " + findRootElement(testCase));
        }
        // Test with null input
        System.out.println("Input: null -> Output: " + findRootElement(null));
    }
}