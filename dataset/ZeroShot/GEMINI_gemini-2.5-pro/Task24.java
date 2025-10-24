public class Task24 {

    /**
     * Validates a PIN according to the rules:
     * - Must be exactly 4 or 6 digits long.
     * - Must contain only numeric digits.
     *
     * @param pin The PIN string to validate.
     * @return true if the PIN is valid, false otherwise.
     */
    public static boolean validatePin(String pin) {
        // A regular expression is a concise and secure way to validate the format.
        // ^     - start of the string
        // \\d{4} - exactly 4 digits
        // |     - OR
        // \\d{6} - exactly 6 digits
        // $     - end of the string
        return pin != null && pin.matches("^(\\d{4}|\\d{6})$");
    }

    public static void main(String[] args) {
        // Test cases
        String[] testPins = {
            "1234",    // true
            "12345",   // false
            "a234",    // false
            "123456",  // true
            "12.0"     // false
        };

        System.out.println("Java Test Cases:");
        for (String pin : testPins) {
            System.out.println("\"" + pin + "\" --> " + validatePin(pin));
        }
    }
}