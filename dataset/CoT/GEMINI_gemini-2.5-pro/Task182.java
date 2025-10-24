import java.util.stream.Collectors;

public class Task182 {

    /**
     * Cleans up a user-entered phone number string.
     *
     * @param phoneNumber The raw phone number string.
     * @return A 10-digit NANP number string, or an empty string if the input is invalid.
     */
    public static String cleanPhoneNumber(String phoneNumber) {
        if (phoneNumber == null) {
            return "";
        }

        // 1. Remove all non-digit characters.
        String cleaned = phoneNumber.replaceAll("[^\\d]", "");

        // 2. Handle country code if present.
        if (cleaned.length() == 11 && cleaned.startsWith("1")) {
            return cleaned.substring(1);
        }

        // 3. Check if the result is a 10-digit number.
        if (cleaned.length() == 10) {
            return cleaned;
        }

        // 4. If not, the number is invalid.
        return "";
    }

    public static void main(String[] args) {
        String[] testCases = {
            "+1 (613)-995-0253",
            "613-995-0253",
            "1 613 995 0253",
            "613.995.0253",
            "12345" // Invalid case
        };

        for (String testCase : testCases) {
            String result = cleanPhoneNumber(testCase);
            System.out.println("Input: \"" + testCase + "\", Cleaned: \"" + result + "\"");
        }
    }
}