public class Task182 {

    /**
     * Cleans up user-entered phone numbers.
     * Removes punctuation and the country code (1) if present.
     * @param phoneNumber The raw phone number string.
     * @return A 10-digit phone number string, or an empty string if the input is not a valid NANP number.
     */
    public static String cleanPhoneNumber(String phoneNumber) {
        // Remove all non-digit characters
        String digits = phoneNumber.replaceAll("[^\\d]", "");

        // Check for country code and length
        if (digits.length() == 11) {
            if (digits.startsWith("1")) {
                return digits.substring(1);
            }
        } else if (digits.length() == 10) {
            return digits;
        }

        // Return empty for invalid formats
        return "";
    }

    public static void main(String[] args) {
        String[] testCases = {
            "+1 (613)-995-0253",
            "613-995-0253",
            "1 613 995 0253",
            "613.995.0253",
            "(223) 456-7890"
        };

        for (String testCase : testCases) {
            System.out.println("Input: \"" + testCase + "\", Output: \"" + cleanPhoneNumber(testCase) + "\"");
        }
    }
}