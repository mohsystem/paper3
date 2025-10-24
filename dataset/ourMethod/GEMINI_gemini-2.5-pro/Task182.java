import java.nio.charset.StandardCharsets;

public class Task182 {

    /**
     * Cleans up a user-entered phone number string by removing punctuation
     * and the NANP country code (1) if present.
     *
     * @param phoneNumber The raw phone number string.
     * @return A 10-digit phone number string if the input is a valid NANP number,
     *         otherwise an empty string.
     */
    public static String cleanPhoneNumber(String phoneNumber) {
        if (phoneNumber == null) {
            return "";
        }

        StringBuilder digitsOnly = new StringBuilder();
        for (char c : phoneNumber.toCharArray()) {
            if (Character.isDigit(c)) {
                digitsOnly.append(c);
            }
        }

        String digits = digitsOnly.toString();

        if (digits.length() == 11) {
            if (digits.startsWith("1")) {
                return digits.substring(1);
            }
        } else if (digits.length() == 10) {
            return digits;
        }

        // Return empty string for invalid lengths or invalid country codes
        return "";
    }

    public static void main(String[] args) {
        String[] testCases = {
            "+1 (613)-995-0253",
            "613-995-0253",
            "1 613 995 0253",
            "613.995.0253",
            "2 (123) 456-7890" // Invalid country code
        };

        for (String testCase : testCases) {
            String cleanedNumber = cleanPhoneNumber(testCase);
            System.out.println("Original: \"" + testCase + "\", Cleaned: \"" + cleanedNumber + "\"");
        }
    }
}